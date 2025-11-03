import serial
import paho.mqtt.client as mqtt
import time

# Configurações da porta serial
serial_port = 'COM5'  # Exemplo para Linux. Use 'COM3' para Windows
baud_rate = 9600

# Configurações do broker MQTT
broker_address = "test.mosquitto.org"
broker_port = 1883
topic = "SensorUltrassonicoJG"
client_id = "SensorUltrassonicoJG"

# Cria um cliente MQTT
client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado ao broker MQTT!")
    else:
        print(f"Falha na conexão, código de retorno: {rc}")

def on_disconnect(client, userdata, rc):
    print("Desconectado do broker MQTT!")

client.on_connect = on_connect
client.on_disconnect = on_disconnect


# Conecta ao broker MQTT
client.connect(broker_address, broker_port,60)
client.loop_start()

try:
    # Abre a porta serial
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    print(f"Porta serial {serial_port} aberta. Lendo dados...")

    while True:
        # Lê uma linha da porta serial
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors= 'ignore').strip()
            if line:
                print(f"Dados recebidos da serial: {line}")
                # Publica a mensagem no tópico
                result = client.publish(topic, line)
                result.wait_for_publish()   
                if result.is_published():
                    print("Dados publicados com sucesso!")
                else:
                    print(f"Falha ao publicar os dados. Erro: {result.rc}")
        time.sleep(0.1)

except serial.SerialException as e:
    print(f"Erro ao abrir a porta serial: {e}")
except KeyboardInterrupt:
    print("Programa encerrado pelo usuário.")
finally:
    client.loop_stop()
    client.disconnect()
    if 'ser' in locals() and ser.is_open:
        ser.close()
    print("Conexão MQTT e porta serial fechadas.")


