import serial
import json
import sys

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
                    salvar_em_json(novo_registro)
except serial.SerialException as e:
    print("Erro ao conectar à porta serial:", e)
except KeyboardInterrupt as e:
    sys.exit(1)
