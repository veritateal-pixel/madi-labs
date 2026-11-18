import base64
import math
import random


def input_message():
    message = str(input("Зашифруйте сообщение: "))
    encoded_message = list(message.encode('utf-8'))                             # байты в числа
    return encoded_message

message = input_message()

def is_prime(n: int) -> bool:
  if n <= 1:
    return False
  for i in range(2, int(n**0.5) + 1):
    if n % i == 0:
      return False
  return True

def input_message(name):
    while True:
        pq = int(input(f"Введите {name} простое число: "))

        if is_prime(pq):
            return pq

        print("Число не простое!")


p = input_message('первое')
q = input_message('второе')

n = p*q

def formula_eilera(p, q):
    return (q-1)*(p-1)

def find_e (fi):
    e = []
    for i in range(1, fi):
        if is_prime(i):
            if fi % i != 0:
                e.append(i)
    return e[0]

def find_d(e, fi):
    d = []
    for i in range (1, 1000):
        if i*e % fi == 1:
            d.append(i)
    return d[0]

fi = formula_eilera(p,q)

e = find_e(fi)
print ("e(открытый ключ) = ", e)

d = find_d(e, fi)
print ("d(закрытый ключ) = ", d)

def encrypt_message(encoded_message, e):
    encrypted_list = []
    for i in encoded_message:
        encrypted_char = i**e % n
        encrypted_list.append(encrypted_char)
    encrypted_message = encrypted_list
    return encrypted_message

def encode_message (message):
    encoded_message = '-'.join(map(str,message))
    return encoded_message

encoded_message = encode_message(message)

def decode_message(message):
   decoded_message = list(map(int, message.split('-')))
   return decoded_message


encrypted_message = encrypt_message(message, e)
print("Зашифрованное сообщение: ", encode_message(encrypted_message))


def descript_message(encrypted_message, d):
    descripted_list = []
    for i in encrypted_message:
      descripted_char = i**d % n
      descripted_list.append(descripted_char)
    descripted_message = descripted_list
    return descripted_message

descripted_message = descript_message(encrypted_message, d)
print("Расшифрованное сообщение: ", bytes(descripted_message).decode("utf-8") )
