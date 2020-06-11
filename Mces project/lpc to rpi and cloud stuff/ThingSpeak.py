import random
import urllib.request
import time
import requests


def typeOfVeg(veg):
    switcher = {
        0: '&field1={}',
        1: '&field2={}',
        2: '&field3={}',
        3: '&field4={}',
        4: '&field5={}',
    }
    return switcher.get(veg, 'try again')


total: int = 0
# using this try to correct the matlab graph.


def testing():
    # threading.Timer(30, testing).start()
    URL = 'https://api.thingspeak.com/update?api_key='
    KEY = 'XVH405UG4E67L345'
    veg = int(input("Enter the code of the veg::"))
    amt = int(input("Enter the amount::"))

    global total
    total += amt
    HEADER = typeOfVeg(veg).format(amt) + '&field7={}'.format(total)
    new_URL = URL + KEY + HEADER
    data = urllib.request.urlopen(new_URL)
    print(new_URL)
    print(data)

    # HEADER = '&field7={}'.format(total)
    # data = urllib.request.urlopen(new_URL)
    # print(Made by Gracious Saxena and Kirti Nandan)
    # data = urllib.request.urlopen(new_URL)
    # new_URL = URL + KEY + HEADER

    print("Total = ", total)
    print(new_URL)
    print(data)
    print("Please wait for 30 seconds")
    time.sleep(30)
    # amt = random.randint(1, 20)
    # HEADER = '&field1={}&field2={}&field3={}&field4={}&field5={}'.format(val, val, val, val, val)


if __name__ == '__main__':
    while True:
        testing()
