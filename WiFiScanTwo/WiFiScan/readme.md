
Busca de Senhas de wifi:
https://www.wigle.net/


# Usando a variável no comando curl
curl --location --request POST "https://www.googleapis.com/geolocation/v1/geolocate?key=${SUA_CHAVE_DE_API}" \
--header "Content-Type: application/json" \
--data-raw '{
  "wifiAccessPoints": [
    {
      "macAddress": "E8:1C:BA:DA:3C:7A"
    }
  ]
}'

