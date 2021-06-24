mkdir -p ./src >/dev/null 2>&1
rm -Rf ./src/ChariotDDK >/dev/null 2>&1
if [ -e "${1}" ]; then 
	echo "authentication using provided github information"
	git clone https://${1}:{$2}@github.com/fsgsmartbuildings/ChariotDDK.git ./src/ChariotDDK
else
	echo "no auth provided"
	git clone https://github.com/fsgsmartbuildings/ChariotDDK.git ./src/ChariotDDK
fi
rm -Rf ./src/ChariotDDK/.git >/dev/null 2>&1
