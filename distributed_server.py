from time import sleep
from pigpio_dht import DHT22
import json
import sys

def initialize_sensors(config_number):
  file = open(f"config{config_number}.json")
  config = json.load(file)
  file.close()
  return config

if len(sys.argv) < 2:
  print('Usage: python3 distributed_server.py <config-id>')
  exit(1)

devices = initialize_sensors(sys.argv[1])
sensor = DHT22(devices['temperature_humidity_sensor'])
print(sensor.read())
