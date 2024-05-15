# Google Geolocation


curl --location --request POST "https://www.googleapis.com/geolocation/v1/geolocate?key=${SUA_CHAVE_DE_API}" \
    --header "Content-Type: application/json" \
    --data '{ "wifiAccessPoints": [{ "macAddress": "d4:92:5e:94:c6:dd"    }]}'


WebSite Postabck
https://webhook.site