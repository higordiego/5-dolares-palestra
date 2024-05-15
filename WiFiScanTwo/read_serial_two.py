import os
import requests
import serial
import json
import sys
from unidecode import unidecode


def tratar_caracteres_especiais(texto):
    # Converte caracteres Unicode para ASCII aproximado
    return unidecode(texto)

def solicitar_geolocalizacao_e_endereco(mac_address, channel, rssi):
    chave_api = os.getenv('SUA_CHAVE_DE_API')
    print(mac_address, channel, rssi)

    if not chave_api:
        print("Chave de API não encontrada na variável de ambiente SUA_CHAVE_DE_API.")
        return None

    # Consulta de geolocalização
    geolocalizacao_url = f'https://www.googleapis.com/geolocation/v1/geolocate?key={chave_api}'

    headers = {
        'Content-Type': 'application/json',
    }

    data = {
        'wifiAccessPoints': [
            { 
                "macAddress": mac_address,
                "signalStrength": rssi,
                "channel": channel,
                'macAddress': mac_address,
            }
        ],
    }

    try:
        response = requests.post(geolocalizacao_url, headers=headers, json=data)
        response.raise_for_status()
        resultado_geolocalizacao = response.json()
    except requests.exceptions.RequestException as e:
        print(f"Erro na solicitação de geolocalização: {e}")
        return None

    print(resultado_geolocalizacao)
    if 'location' not in resultado_geolocalizacao:
        print("Não foi possível obter a geolocalização.")
        return None

    # Obtenção das coordenadas lat/lng
    latitude = resultado_geolocalizacao['location']['lat']
    longitude = resultado_geolocalizacao['location']['lng']

    # Consulta reversa para obter o endereço
    endereco_url = f'https://maps.googleapis.com/maps/api/geocode/json?latlng={latitude},{longitude}&key={chave_api}'

    try:
        response = requests.get(endereco_url)
        response.raise_for_status()
        resultado_endereco = response.json()
    except requests.exceptions.RequestException as e:
        print(f"Erro na solicitação de endereço: {e}")
        return None

    if 'results' not in resultado_endereco or not resultado_endereco['results']:
        print("Não foi possível obter o endereço.")
        return None

    primeiro_resultado = resultado_endereco['results'][0]
    endereco = primeiro_resultado.get('formatted_address', 'Endereço não disponível')

    # Tratar caracteres especiais no endereço
    endereco_tratado = tratar_caracteres_especiais(endereco)

    return {
        'latitude': latitude,
        'longitude': longitude,
        'endereco': endereco_tratado,
    }

# Porta serial onde o ESP32 está conectado
porta_serial = '/dev/ttyUSB0'  # Altere para a porta serial correta

# Configurações da porta serial
baud_rate = 19200  # Taxa de transmissão de dados

# Arquivo JSON para armazenar os registros
json_file_path = 'db.json'

def ssid_ja_existe(registros, ssid):
    for registro in registros:
        if registro.get("SSID") == ssid:
            return True
    return False

# Inicializar a lista de registros a partir do arquivo JSON, se existir
try:
    with open(json_file_path, 'r') as json_file:
        registros_existentes = json.load(json_file)
except FileNotFoundError:
    registros_existentes = []

def salvar_em_json(novo_registro):
    registros_existentes.append(novo_registro)
    with open(json_file_path, 'w') as json_file:
        json.dump(registros_existentes, json_file, indent=2)

try:
    # Inicializa a comunicação serial
    ser = serial.Serial(porta_serial, baud_rate, timeout=1)
    print("Conexão estabelecida com sucesso!")

    # Loop para ler continuamente da porta serial e imprimir os dados do stdout
    while True:
        # Ler uma linha da porta serial
        line = ser.readline().decode('utf-8').strip()
        # print(line)
        
        # Imprimir os dados do stdout
        # print("Dados do stdout:", line)

        # Se a linha contiver informações de rede Wi-Fi, separar e imprimir
        if line:
            # Separar as informações usando espaços como delimitadores
            data = line.split()
            data = [elemento for elemento in data if elemento != '|']            

            if not line.startswith("Nr"):
            # Verificar se o número do registro já existe no arquivo JSON antes de salvar
                novo_registro = {
                    "Nr": data[0],
                    "SSID": data[1],
                    "BSSID": data[2],
                    "RSSI": data[3],
                    "CH": data[4],
                    "Encryption": data[5]
                }

                if novo_registro not in registros_existentes and not ssid_ja_existe(registros_existentes, data[1]):
                    # Chamada da função
                    mac_address = novo_registro['BSSID']
                    print(novo_registro)
                    location = solicitar_geolocalizacao_e_endereco(mac_address, novo_registro['CH'], novo_registro['RSSI'])
                    novo_registro['location'] = location 
                    salvar_em_json(novo_registro)
except serial.SerialException as e:
    print("Erro ao conectar à porta serial:", e)
except KeyboardInterrupt as e:
    sys.exit(1)

 