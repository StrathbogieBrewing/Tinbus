import serial
import time
 
class term:

    def __init__(self, serialPort):
        self.ser = serial.Serial(
            port=serialPort,
            baudrate=19200,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE,
            parity=serial.PARITY_NONE,
            timeout=1)  # default timeout for reading in seconds 
        time.sleep(0.2) # allow time for initialisation of port to complete 
        
    def deframe(self, frame):
        msg = bytearray()
        source = iter(frame)
        csum = 0  
        # ch = next(source, 0)
        # if ch == ord(":"):
        ch = next(source, 0)
        while ch != 0:
            if ch == ord("\r"):
                ch = next(source, 0)
                if ch == ord("\n"):
                    break
                if ch != 0x1b:
                    msg = bytearray()
                    break
                ch = next(source, 0)
                if ch != ord("["):
                    msg = bytearray()
                    break
                ch = next(source, 0)
                if ch != ord(">"):
                    msg = bytearray()
                    break
                ch = next(source, 0)
                if (ch - ((csum >> 4) & 0x07)) != ord("0"):
                    msg = bytearray()
                    break
                ch = next(source, 0)
                if (ch - (csum & 0x0F)) != ord("A"):
                    msg = bytearray()
                    break
                ch = next(source, 0)
                if ch != ord("\n"):
                    msg = bytearray()
                    break
                break
            csum = csum + ch
            msg.append(ch)
            ch = next(source, 0)    
            if ch == 0:
                msg = bytearray()
        return msg

    def enframe(self, msg):
        frame = bytearray()
        csum = 0
        for ch in msg:
            csum = csum + (ch)
            frame.append(ch)
        frame.append(ord("\r"))
        frame.append(0x1B)
        frame.append(ord("["))
        frame.append(ord(">"))
        frame.append(ord("0") + ((csum >> 4) & 0x07))
        frame.append(ord("A") + (csum & 0x0F))
        frame.append(ord("\n"))

        return frame 

    def send(self, msg):
        self.ser.flushOutput()
        self.ser.flushInput()
        
        tries = 3
        
        send_data = self.enframe(msg)
        self.ser.write(send_data)
        echo_data = self.ser.readline()
        #resp = None
        if echo_data == send_data:
            resp_data = self.deframe(self.ser.readline())
            # print(echo_data)
            # print(resp_data)
        else:
            resp_data = None
    #    ser.close()
        return resp_data

    def getvalue(self, message):
        import re
        # self.send(message)
        # time.sleep(2) # wake up meter from power down
        tries = 3
        while(tries): 
            tries -= 1
            response = self.send(message)
            if response.startswith(message[:-1] + b' '):
                response = response.replace(message[:-1] + b' ', b'')
                print(response)
                try:
                    retval = float(response)
                    return retval
                except ValueError:
                    pass
            tries -= 1       
            time.sleep(0.5)
        return None  
            
        
    def setvalue(self, message):
        #message = bytearray(msg, 'utf-8')
        response = self.send(message)
        #print(message)
        #print(response)  
        if message == response:
            return message  
        else:
            return None

    def release(self):
        self.ser.close()

PORT = '/dev/ttyUSB0'

terminal = term(PORT)

response = terminal.getvalue(b'ut61ac:meas:volt:ac?')

print(response)
