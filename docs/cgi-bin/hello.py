#!/usr/bin/python 
import os
   
para = os.environ.get('HTTPD_PARAMETER')
[key,value] = para.split("=",1)
 
print('<html>')
print('<body>')
print('<h1>' + '你好,' + value + '!' + '</h1>')
print('<p>' + '如果不能再见到你，那么祝福你早安，午安，晚安！'+'</p>')    
print('</body>')
print('</html>')
