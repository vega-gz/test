#! /usr/bin/env python
# coding: utf-8

from multiprocessing import Process, Manager
from time import sleep
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
import serial
import re

PORT_NUMBER = 8080

def find_tty (pv, infp, fport):
  time.sleep(1)
  found = False
  listport = glob.glob('/dev/tty[a-zA-Z]*')
  for port in listport:
    try :
      pv.value = port
      ser = serial.Serial(port)
      ser.baudrate = 115200
      time.sleep(0.5)
      ser.write("AT \r\n")
      while infp.value == 0:
        time.sleep (0.1)
      if infp.value == 1:
        print "Port " + port + " modem"
        fport.value = port
      elif infp.value == 2:
        print "timeout port : " + port
      infp.value = 0
      found = True
    except serial.serialutil.SerialException :
      pass

  if not found :
    print "Последовательных портов не обнаружено"

def read_port (pv, infp, fport):
  lastport = ''
  while True:
    if pv.value != 0 and pv.value != lastport :
      try:
        lastport = pv.value
        port_modem = pv.value
        ser1 = serial.Serial(port_modem)
        ser1.baudrate = 115200
        ser1.timeout = 3
        port_at = False
        t = time.time()
        timeset = True

        while timeset:
            list2 = ser1.readline()
            if len(list2) != 0:
              if list2.startswith('OK') == True:
                infp.value = 1
                timeset = False
            if time.time() - t >= 2.5: #time is over send of port
                port_at = False
                infp.value = 2
                timeset = False
      except serial.serialutil.SerialException :
        pass

def read_cops(dict, dict_data_send, v, csq1, fport):
  while True:
    if len(fport.value) != "":
      ser = serial.Serial(fport.value)
      ser.baudrate = 115200
      list2 = ''
      #print (ser.readline())
      while list2.startswith('OK') == False :
          list2 = ser.readline()
          #print list2
          if list2.startswith('+COPS'):
            for ln in list2.split('('):
              ln1 = "\'"+ ln + "\'"
              #print ln1
              r1 = re.search('.*\"([a-zA-Z0-9_ ]{1,})\".*\"([0-9]{3,})\"', ln)
              #r1 = re.search('.*[a-zA-Z0-9_ ]{1,}.*', ln1)
              if r1:
                 dict[r1.group(1)] = r1.group(2)
              else:
                print "not find RE"
              #print r1.group(1)
              #print r1.group(2)

              #if r1.group(1) & r1.group(2) != null
               #dict[r1.group(1)] = r1.group(2)
          elif list2.startswith('OK'):
            v.value = 1
          elif list2.startswith('ERROR'):
            v.value = 2
          elif list2.startswith('+ZEND') or list2.startswith('\r\n'):
            v.value = 3
          elif list2.startswith('+CREG'):
            print (list2)
            r2 = re.search('.*([0-2]{1}),([0-5]{1})', list2)
            if r2:
                 print (r2.group(1))
                 print (r2.group(2))
            else:
                 v.value = 6
          elif list2.startswith('+CSQ'):
            v.value = 4
            print ('value_oper ' + list2)
            csq1.append(list2)
            #dict_data_send[test2] = list2
          else:
            trash = 0
            #print "not COPS by this: " + list2
      #print dict

      # list operators
      #for key, value in dict_data_send.items():
      #    print(key, value)
      #time.sleep(10)
      #for key, value in dict.items():
      #    print(key, value)
      #pin.send(dict)

def write_serial(dict, dict_data_send, v, fport):
  while True:
    if len(fport.value) != "":
      c_err = 0
      c_ok = 0
      #dict = {}
      print "write AT code  Cops=? \r\n"
      #ser.open()
      if v.value == 0:
        ser.write("AT+COPS=? \r\n")
        v.value = 8
        #sleep(20.0)
     #ser.write("AT#SERVINFO \r\n")
     #time.sleep(5.0)
      #ser.close()
      if c_err == 1:
        print("Poll false: dict not send")
      else:
      #  dict = pout.recv()
        print ("len dict")
        print (len(dict))
      if len(dict) != 0:
        for key, value in dict.items():
          value_oper = key
          print(" send cops code in connect")
          print (value_oper)
          strwrite = 'at+cops=1,2,' + value + ' \r\n'
          ser.write(strwrite)
         #print strwrite
           # do..while in python
          request_error = 0
          while True:
            request_error = request_error + 1
            sleep(10.0)
            ser.write("AT+CREG? \r\n")
            if request_error >= 5:
              break
          sleep(1.0)
          ser.write("AT+CSQ \r\n")
          #ser.write("AT#MONI \r\n")
      print ("not len in dict")
      sleep(3.0)

def webserv1(dict, csq1, fport):
  class myHandler(BaseHTTPRequestHandler):

          #Handler for the GET requests
          def do_GET(self):
                  self.send_response(200)
                  #self.send_header('Content-type','text/html')
                  #self.end_headers()
                  # Send the html message
                  #self.wfile.write("inf operstors! \r\n")
                  #self.wfile.write(dict)
                  self.send_header("Content-type", "text/html")
                  self.end_headers()
                  self.wfile.write("<html><head><title>Title goes here.</title></head>")
                  self.wfile.write("<body><p>")
                  #button
                  self.wfile.write("<button>test button</button>")
                  self.wfile.write(dict)
                  self.wfile.write("<p>force basec.</p>")
                  self.wfile.write(csq1)
                  self.wfile.write("</p>")
                  # If someone went to "http://something.somewhere.net/foo/bar/",
                   # then s.path equals "/foo/bar/".
                  self.wfile.write("<p>You accessed path: %s</p>" % self.path)
                  self.wfile.write("</body></html>")
                  return

  try:
          #Create a web server and define the handler to manage the
          #incoming request
          server = HTTPServer(('', PORT_NUMBER), myHandler)
          print 'Started httpserver on port ' , PORT_NUMBER

          #Wait forever for incoming htto requests
          server.serve_forever()

  except KeyboardInterrupt:
          print '^C received, shutting down the web server'
          server.socket.close()
    

if __name__ == '__main__':
    with Manager() as manager:
        csq1 = manager.list()
        dict_data_send = manager.dict()
        dict = manager.dict()
        d = manager.dict()
        l = manager.list(range(10))
        v = manager.Value('i', 0)
        d1 = d
       
        pv = manager.Value('i', 0)
        infp = manager.Value('i', 0)
        fport = manager.Value('i', "")

        p1 = Process(target=webserv1, args=(dict, csq1, fport))
        p2 = Process(target=read_cops, args=(dict, dict_data_send, v, csq1, fport))
        p3 = Process(target=write_serial, args=(dict, dict_data_send, v, fport))
        p4 = Process(target=find_tty, args=(pv, infp, fport))
        p5 = Process(target=read_port, args=(pv, infp, fport))

        p1.start()
        p2.start()
        p3.start()
        p4.start()
        p5.start()

        p1.join()
        p2.join()
        p3.join()
        p4.join()
        p5.join()

