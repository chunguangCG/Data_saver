import asyncio as aio
import socket
import struct
from signal import SIGINT
from functools import partial
import time


class Serializer(object):
    def __init__(self, file_name):
        self._file_name = file_name
        self._hand_data_list = []
        self._jr3_data_list = []
        self._jr3_ip = '192.168.0.2'
        self._hand_connected = False
        self._jr3_connected = False
        self._file = open(self._file_name, mode='w+')
    def connected(self, ip):
        if ip == self._jr3_ip:
            self._jr3_connected = True
        else:
            self._hand_connected = True
    def disconnected(self, ip):
        if ip == self._jr3_ip:
            self._jr3_connected = False
        else:
            self._hand_connected = False
    def append_data(self, data, ip):
        if self._jr3_connected and self._hand_connected:
            if ip == self._jr3_ip:
                self._jr3_data_list.append(data)
            else:
                self._hand_data_list.append(data)
                if len(self._jr3_data_list) > 1:
                    hand_fmts='f'*30
                    jr3_fmts='f'*12
                    hand_data = self._hand_data_list[-1]
                    jr3_data  = self._jr3_data_list[-1]
                    hand = struct.unpack(hand_fmts, hand_data)
                    jr3 = struct.unpack(jr3_fmts, jr3_data)
                    str_ = ['{:.3f}'.format(h) for h in hand] + ['{:.3f}'.format(j) for j in jr3]
                    self._file.write('\t'.join(str_) + '\n')
                    print('{0}: save one frame, size: {1}'.format(time.time(), len(self._jr3_data_list[-1])))
                    self._hand_data_list, self._jr3_data_list = [], []
class DataProtocol(aio.Protocol):
    def __init__(self, loop, serializer):
        self._loop = loop
        self._serializer = serializer
    def connection_made(self, transport):
        self._transport = transport
        self._peer_name = self._transport.get_extra_info('peername')
        self._serializer.connected(self._peer_name[0])
        print('connected by {0}'.format(self._peer_name))
    def data_received(self, data):
        self._serializer.append_data(data, self._peer_name[0])
    def connection_lost(self, exc):
        self._serializer.disconnected(self._peer_name[0])
        print('disconnected by {0}'.format(self._peer_name))

def start_server(ip, port):
    serializer = Serializer('data.txt')
    loop = aio.get_event_loop()
    coro = loop.create_server(lambda: DataProtocol(loop, serializer), ip, port)
    loop.create_task(coro)
    try:
        loop.run_forever()
    except KeyboardInterrupt:
	    print('program quit')

if __name__ == '__main__':
    print('program started')
    start_server('192.168.0.1', 18080)
