#ifndef CHARIOT_DDK_HTTP_ACCESS_POINT_DEVICE_H
#define CHARIOT_DDK_HTTP_ACCESS_POINT_DEVICE_H

#include <cstdlib>
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <exception>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional/optional.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "md5.h"
#include "accessPointBase.h"
#include "httplib.h"

using namespace Chariot::DDK::Crypto;

namespace Chariot {
	namespace DDK {
		namespace Devices {

			class HTTPAccessPoint {
				public:
					HTTPAccessPoint( const char *ip_address, const int port, const char *user, const char *passwd, const char *type );
					HTTPAccessPoint( ptree& access_point_entry );

					bool getClients( ptree& devicesOut );

					std::string getType() { return this->type; }
					std::string getIp() { return this->ip; }
					int getPort() { return this->port; }
					std::string getUser() { return this->username; }

					bool load( ptree& data );

				private:
					std::string ip;
					int port;
					std::string username;
					std::string hash;
					std::string type;

					std::string referer;
					
					std::string cookie_url = "/";
					std::string login_url = "/";
					std::string status_url = "/data/status.client.user.json?operation=load";
					std::string freq_url = "/data/status.wireless.radio.json?operation=read&radioID=";
					
					bool get_cookie( std::string& out_cookie, std::string& outerror );
					bool do_login( std::string& in_cookie, std::string& outerror );
					bool get_clients( std::string& in_cookie, ptree& clients, std::string& outerror );
					bool get_radio_freq( std::string& in_cookie, int radio_id, std::string& out_freq, std::string& outerror );
					bool ping_device( std::string ip_address, std::string &outerr );
			};

		}
	}
}

#endif