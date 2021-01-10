#!/usr/bin/python 
# -*- coding:utf-8 -*-

import os
import urllib.parse
   
para = os.environ.get('HTTPD_PARAMETER')
[key,value] = para.split("=",1)
value = urllib.parse.unquote(value)

print('<html>')
print('<body>')
print('<h1>' + '你好,' + value + '!' + '</h1>')
print('<p>' + '如果不能再见到你，那么祝福你早安，午安，晚安！'+'</p>')    
print('</body>')
print('</html>')
