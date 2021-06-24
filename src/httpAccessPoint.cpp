
#include "./include/httpAccessPoint.h"

using namespace Chariot::DDK::Devices;
using namespace Chariot::DDK::Crypto;
  


HTTPAccessPoint::HTTPAccessPoint( const char *ip_address, const int port, const char *user, const char *passwd, const char* type ) {
	this->ip = ip_address;
	this->port = port;
	this->username = user;
	this->type = type;
	
	std::string pass = passwd;
	MD5 md5 = MD5( pass );
	this->hash = md5.hexdigest();

	this->referer = "http://" + this->ip + "/";
}

HTTPAccessPoint::HTTPAccessPoint( ptree& access_point_entry ) {
	std::string password;

	this->ip = access_point_entry.get<std::string>("ip");

	boost::optional<int> port_child = access_point_entry.get_optional<int>( "port" );
	if( !port_child ) this->port = 80;
	else this->port = port_child.get();

	boost::optional<std::string> user_child = access_point_entry.get_optional<std::string>( "user" );
	if( !user_child ) this->username = "";
	else this->username = user_child.get();

	boost::optional<std::string> pass_child = access_point_entry.get_optional<std::string>( "passwd" );
	if( !pass_child ) password = "";
	else password = pass_child.get();

	MD5 md5 = MD5( password );
	this->hash = md5.hexdigest();
	this->referer = "http://" + this->ip + "/";
}

bool HTTPAccessPoint::load( ptree& data ) {

	boost::optional<std::string> ip_value = data.get_optional<std::string>( "ip" );
	if( ip_value )
		this->ip = ip_value.get();

	boost::optional<int> port_value = data.get_optional<int>( "port" );
	if( port_value )
		this->port = port_value.get();

	boost::optional<std::string> user_value = data.get_optional<std::string>( "user" );
	if( user_value )
		this->username = user_value.get();

	boost::optional<std::string> passwd_value = data.get_optional<std::string>( "passwd" );
	if( passwd_value ) {
		MD5 md5 = MD5( passwd_value.get() );
		this->hash = md5.hexdigest();
	}

	boost::optional<std::string> type_value = data.get_optional<std::string>( "type" );
	if( type_value ) {
		this->type = type_value.get();
	}

	return true;
}

bool HTTPAccessPoint::getClients( ptree& outData ) {
	std::string cookie;
	std::string error;

	std::string radio0_freq;
	std::string radio1_freq;

	if( !this->get_cookie( cookie, error ) ){
		std::cerr << error << std::endl;
		return false;
	}

	if( !this->do_login( cookie, error ) ){
		std::cerr << error << std::endl;
		return false;
	}

	if( !this->get_radio_freq( cookie, 0, radio0_freq, error ) ){
		std::cerr << error << std::endl;
		return false;
	}

	if( !this->get_radio_freq( cookie, 1, radio1_freq, error ) ){
		std::cerr << error << std::endl;
		return false;
	}

	ptree data;
	if( !this->get_clients( cookie, data, error ) ){
		std::cerr << error << std::endl;
		return false;
	}

	bool success = data.get<bool>("success");
	if( !success ) {
		int error_num = data.get<int>("error");
		std::string error = "[ap::getClients][" + this->ip + "] data was retrieved but the ap reported an error: " + std::to_string(error_num);
		std::cerr << error << std::endl;
		return false;
	}

	BOOST_FOREACH( boost::property_tree::ptree::value_type &v, data.get_child("data") ) {
		ptree entry;
		std::string mac = v.second.get<std::string>("MAC");
		std::replace(mac.begin(), mac.end(), '-', ':');
		std::transform(mac.begin(), mac.end(), mac.begin(), ::toupper);

		entry.put<std::string>( "hostname", v.second.get<std::string>("hostname") );
		entry.put<std::string>( "signal", v.second.get<std::string>("RSSI") );
		entry.put<std::string>( "ip", v.second.get<std::string>("IP") );

		int radio_id = v.second.get<int>("Radio");
		entry.put<int>( "radio", radio_id );

		if( radio_id == 0 )
			entry.put<std::string>( "freq", radio0_freq );

		if( radio_id == 1 )
			entry.put<std::string>( "freq", radio1_freq );

		entry.put<std::string>( "noise", "unk" );
		entry.put<std::string>( "type", "802.11" );
		entry.put<std::string>( "source", this->ip );

		entry.put<bool>( "alive", this->ping_device( this->ip, error ));
		outData.push_back( std::make_pair( mac, entry ) );
	}
	return true;
}

bool HTTPAccessPoint::get_cookie( std::string& out_cookie, std::string& outerror ){
	out_cookie.clear();
	outerror.clear();

	httplib::Client cli( this->ip.c_str(), this->port );

	httplib::Headers headers = {
		{ "Referer", this->referer.c_str() }
	};
	
	auto res = cli.Get( this->cookie_url.c_str() );
	if( res ) {
		if ( !res->has_header( "Set-Cookie" )  ) {
			outerror = "no cookie provided";
			return false;
		}
	} else {
		std::cerr << res.error() << std::endl;
		outerror = "error getting cookie";
		return false;
	}

	out_cookie = res->get_header_value( "Set-Cookie" );
	out_cookie = out_cookie.substr(0, out_cookie.find( std::string(";") ) );
	std::cout << out_cookie << std::endl;
	return (res->status == 200);	
}

bool HTTPAccessPoint::do_login( std::string& in_cookie, std::string& outerror ){
	outerror.clear();
	std::string url = "/";

	httplib::Client cli( this->ip.c_str(), this->port );
	httplib::Headers headers = {
		{ "Referer", this->referer.c_str() },
		{ "Cookie", in_cookie.c_str() }
	};

	std::string data = "username=" + this->username + "&password=" + this->hash;
	auto res = cli.Post( this->login_url.c_str(), headers, data, "application/x-www-form-urlencoded" );

	if( res ) {
		std::cout << res->body << std::endl;
		return (res->status == 200);
	} else {
		std::cerr << res.error() << std::endl;
		outerror = "error doing login";
		std::cerr << outerror << std::endl;
		return false;
	}
}

bool HTTPAccessPoint::get_clients( std::string& in_cookie, ptree& data, std::string& outerror ) {
	outerror.clear();

	httplib::Client cli( this->ip.c_str(), this->port );
	httplib::Headers headers = {
		{ "Referer", this->referer.c_str() },
		{ "Cookie", in_cookie.c_str() }
	};
	
	auto res = cli.Get( this->status_url.c_str(), headers );

	if( res ) {
		std::istringstream response_stream( res->body );
		std::cout << res->body << std::endl;
		try {	
			read_json( response_stream, data );
		}
		catch(const std::exception& e) {
			outerror = e.what();
			outerror = "[ap::get_clients][" + this->ip + "] " + outerror;
			return false;
		}
		return (res->status == 200);
	} else {
		std::cerr << res.error() << std::endl;
		outerror = "error getting clients";
		std::cerr << outerror << std::endl;
		return false;
	}
}

bool HTTPAccessPoint::get_radio_freq( std::string& in_cookie, int radio_id, std::string& out_freq, std::string& outerror ) {
	outerror.clear();

	std::string host = "http://" + this->ip + ":" + std::to_string( this->port );
	std::string url = this->freq_url + std::to_string(radio_id);

	httplib::Client cli( this->ip.c_str(), this->port );
	httplib::Headers headers = {
		{ "Referer", this->referer.c_str() },
		{ "Cookie", in_cookie.c_str() }
	};
	
	auto res = cli.Get( url.c_str(), headers );
	if( res ) {
		std::istringstream response_stream( res->body );
		std::cout << res->body << std::endl;
		ptree data;
		try {	
			read_json( response_stream, data );
		}
		catch(const std::exception& e) {
			outerror = e.what();
			outerror = "[ap::get_clients][" + this->ip + "] " + outerror;
			return false;
		}

		out_freq = data.get<std::string>("data.channel");
		out_freq = out_freq.substr(out_freq.size()-7, out_freq.size()-3);
		
		return (res->status == 200);
	} else {
		std::cerr << res.error() << std::endl;
		outerror = "error getting radio information";
		std::cerr << outerror << std::endl;
		return false;
	}
}

bool HTTPAccessPoint::ping_device( std::string ip_address, std::string &outerr ) {
	outerr.clear();

	std::string ping_cmd = "ping -c1 -s1 " + ip_address + " > /dev/null 2>&1";
	try{
		int x = system( ping_cmd.c_str() );
		return (x==0);
	} catch( const std::exception& ex ) {
		std::string err = ex.what();
		outerr = err;
	}
	return false;
}




