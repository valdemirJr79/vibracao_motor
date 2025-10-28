import serial
import time
import paho.mqtt.client as mqtt

serial_port=COM3
baud_rate = 9600
broker_addres = "broker.hivemq.com"
broker_port = 1883
topic = "iot/sala1/temperatura"
client_id = "Python-Serial_Publisher"

client=mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado ao broker MQTT!")
    else:
        print(f"Falha na conexão, código de retorno:{rc}")

def on_disconnect(client, userdata, rc):
    print("Desconectado ao broker MQTT!")

    client.on_connect = on_connect
    client.on_disconnect =on_disconnect

client.connect(broker_addres, broker_port, 60)
client.loop_start()

try
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    print(f"Porta Serial {serial_port} aberta. Lendo dados...")

while True:
    if ser.in_waiting>0:
        line = ser.readline().decode('utf-8').strip()
        if line:
            print(f"Dados recebidos de serial:{line}")
        result = client.publish(topic, line)
        result.wait_for_publish()
        if result.is_published():
            print("Dados publicados.")
        else:
            print(f"Falha ao publicar os dados. Erro:{result.rc}")
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
