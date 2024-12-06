import serial, time, os
import matplotlib.pyplot as p
import openpyxl as xl
from openpyxl.chart import ScatterChart, Reference, Series
#Importamos las librerías para utilizar el programa:
# ejecutar:
# pip install pyserial
# pip install matplotlib
# pip install openpyxl

xlName = "CodeChemSat_Data.xlsx" #Nombre de nuestro archivo de datos a guardar, debe terminar con la extensión xlsx

if (os.path.exists(xlName)): #¿el archivo ya existe?
    print("Cuidado, un registro de actividad satelital ya existe en este directorio")
    print("Asegurese de mantener sus registros previos en la carpeta 'test'")
    print("¿Desea eliminar el archivo en este directorio para iniciar un nuevo registro? (Y/N)")
    if (input().lower()!="y"): #si el usuario ingresa cualquier otra cosa que no sea "y", se detiene el programa.
        print("\nDeteniendo registro...")
        exit()
        

port="COM3" #este es el puerto a donde está conectado el receptor/estación terrena
#COM3, COM6, COM11, son algunos de los puertos que varían de computadora a computadora

s = serial.Serial(port,9600) # Se puede verificar el puerto en el IDE de arduino, y el segundo datos es el baudrate seteado por código (Serial.begin(9600);)

excel = xl.Workbook() 
celdas = excel.active
# variables para el uso de la librería openpyxl (excel)
try:
    excel.save(xlName)
except PermissionError: 
    
    print("Error, Cierre el archivo Excel para iniciar")
    exit()
# va a intentar guardar un archivo vacío, si da un error de permiso, lo más probable es que el archivo excel esté abierto.


# A partir de acá empieza a crear el modelo para poder interpretar mejor los datos
j=0 # variable auxiliar
lista = ["Tiempo","Temperatura","Presion","Altura","Metano","Calidad","Presion SNM: "]
for i in "ABCDEFG":
    celdas[f"{i}1"] = lista[j]
    j+=1

celdas["G2"]="Latitud:"
celdas["G3"]="Longitud:"

p.ion() #p, fig, axs son variables referidas a la creación y actualización de los gráficos


fig, axs = p.subplots(5) # se utilizarán 5 gráficos, por lo que pasamos por argumento un 5.

p.subplots_adjust(top=0.93, bottom=0.08, left=0.03, right=0.99, hspace=0.4,wspace=0.0)
# configuramos los márgenes de la ventana de los gráficos.

#creamos una lista para cada dato que vamos a recibir y poder agregarlos al final de la lista luego.
x = [] #x se refiere al tiempo transcurrido, no es un dato recibido.
lTemp = []
lPresion = []
lMetano = []
lCalidad = []
lAltura = []
AltInit = 0.0 #como altura inicial es una constante, la iniciamos como float

t0 = time.time() #t0 se refiere a la hora en la que se inició este programa
cont = 1 #este contador auxiliar nos servirá más tarde para poder ajustar los datos del excel

while True:
    x.append(time.time() - t0) #a la lista x, se la agrega al final el tiempo "actual", que es la diferencia entre el tiempo en este instante y el inicial
    try:
        d = s.readline().decode().rstrip() #va a intentar recibir y decodificar los datos del serial del receptor.
    except:
        print("Advertencia, receptor desconectado.")
        print("10 segundos para volver a conectar")
        time.sleep(10) # si falla, va a intentarlo de nuevo 10 segundos después
        try:
            s = serial.Serial(port,9600)
            d = s.readline().decode().rstrip()
            
        except: #si la estación terrena sigue desconectada 10 segundos después va a terminar el loop y proceder con el guardado de datos.
            print("Timeout, apagando...")
            break
            
    if d!=0: #comprobación para que intercepte datos vacíos
        cont+=1
        
        valor = d.split(",") # va a separar cada valor por una coma
        try: 
            lTemp.append(float(valor[0]))
            lPresion.append(float(valor[1]))
            lAltura.append(float(valor[2]))
            lMetano.append(float(valor[3]))
            lCalidad.append(float(valor[4]))
            #el primero es la temperatura, segundo la presión, tercero la altura y así sucesivamente, este orden está programado en el receptor 
            
            celdas[f"A{cont}"] = time.time() - t0
            celdas[f"B{cont}"] = float(valor[0]) # temp
            celdas[f"C{cont}"] = float(valor[1]) # presion
            celdas[f"D{cont}"] = float(valor[2]) # altura
            celdas[f"E{cont}"] = float(valor[3]) # metano
            celdas[f"F{cont}"] = float(valor[4]) # calidad
            
            celdas["H1"] = valor[5] # presion inicial
            celdas["H2"] = valor[6] # latitud
            celdas["H3"] = valor[7] # longitud
            latitud = valor[6]
            longitud = valor[7]
        except:
            continue
        # si esto da error, cosa que pasa normalmente al iniciarse por datos basura del serial, va a ignorarlo y saltar a la próxima instancia del bucle
        

        fig.text(0.02,0.95,f"Presion SNM: {valor[5]}") # en el gráfico se escribe el valor de la presión inicial
        
        for ax in axs: # para cada gráfico
            
            
            # temperatura
            
            axs[0].clear()
            axs[0].set_title("Temperatura °C")
            axs[0].plot(x,lTemp, color="b")
            
            
            # presion
            axs[1].clear()
            axs[1].set_title("Presion mBar")
            axs[1].plot(x,lPresion, color="r")
            
            # altura
            axs[2].clear()
            axs[2].set_title("Altura M")
            axs[2].plot(x,lAltura, color="g")
            
            # metano
            axs[3].clear()
            axs[3].set_title("Metano PPM")
            axs[3].plot(x,lMetano, color="k")
            
            # calidad aire
            axs[4].clear()
            axs[4].set_title("Calidad del Aire PPM")
            axs[4].plot(x,lCalidad, color="c")
            
            #para cada gráfico, primero se borra completamente, para luego volverle a pasar la lista actualizada
            # y que simule la actualización en tiempo real, imperceptible para el espectador y logra el efecto deseado
            
            p.pause(0.01)
        
              
# ------------ guardar grafico ----------
excel.save(xlName) 
print("holis") #Mensaje de prueba para saber que se ha terminado el bucle

graphx = Reference(celdas,min_col=1,min_row=2,max_col=1,max_row=cont)
#el eje x, en todos las gráficas va a ser igual, corresponde al tiempo


grafico = ScatterChart() # Se inicializa un nuevo gráfico de coordenadas
grafico.title="Temperatura" # título
graphy = Reference(celdas,min_col=2,min_row=2,max_col=2,max_row=cont) #le pasamos dónde están los datos que debe agregar al gráfico
#celdas (que se refiere a la hoja de cálculo actual), la columna donde empieza, la fila donde empieza, la columna donde termina, y la fila donde termina

grafico.series.append(Series(graphy,graphx,title="°C")) #esta función "creará el gráfico" virtualmente, le pasamos el eje x y el eje y, le ponemos de título a la línea "°C"


grafico.x_axis.title= "Segundos encendido"


celdas.add_chart(grafico,"J2") #ahora sí, se agrega al excel el gráfico en la celda "J2"


grafico = ScatterChart()
grafico.title="Presión"
graphy = Reference(celdas,min_col=3,min_row=2,max_col=3,max_row=cont) #3
grafico.series.append(Series(graphy,graphx,title="mbar"))
celdas.add_chart(grafico,"J18")


grafico = ScatterChart()
grafico.title="Altura"
graphy = Reference(celdas,min_col=4,min_row=2,max_col=4,max_row=cont) #4
grafico.series.append(Series(graphy,graphx,title="metros"))
celdas.add_chart(grafico,"J34")


grafico = ScatterChart()
grafico.title="CH4/Gas metano"
graphy = Reference(celdas,min_col=5,min_row=2,max_col=5,max_row=cont) #5
grafico.series.append(Series(graphy,graphx,title="ppm"))
celdas.add_chart(grafico,"J50")


grafico = ScatterChart()
grafico.title="Calidad del Aire"
graphy = Reference(celdas,min_col=6,min_row=2,max_col=6,max_row=cont) #6
grafico.series.append(Series(graphy,graphx,title="ppm"))
celdas.add_chart(grafico,"J66")

#lo mismo para todos los gráficos.

excel.save(xlName) # se guarda para asegurarse.

#finalmente, mensajes por consola que indican dónde se encuentra nuestro CANSAT
print("Nuestro CANSAT está en:")
print(f"google.com/maps/place/{latitud},{longitud}")
print("Gracias")
  #Programado y testeado por Oviedo Verónica y Maccari Valentino.
  #Contacto: veroviedo935@gmail.com

