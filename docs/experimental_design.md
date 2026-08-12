# Fase 1 - Comparacion inicial de tecnologias

## Estado del documento

Esta es la especificacion inicial del experimento. No contiene resultados ni afirma que
los escenarios ya esten implementados. Su objetivo es fijar el alcance antes de escribir
los modelos de ns-3.

La Fase 1 cubre los objetivos especificos OE1 y OE2 del plan de tesis:

- simular enlaces inalambricos punto a punto en tuneles mineros;
- registrar las metricas necesarias para comparar tecnologias y preparar el futuro
  conjunto de datos.

La red neuronal, TinyML y el ESP32-C3 pertenecen a las fases posteriores. VLC queda
fuera del alcance actualizado de la tesis.

## Pregunta experimental

¿Cual de las tres tecnologias candidatas satisface mejor los requisitos de un enlace de
telemando punto a punto cuando todas reciben la misma carga de aplicacion y recorren el
mismo escenario subterraneo?

Las tecnologias candidatas son:

1. Wi-Fi de referencia, con perfil objetivo IEEE 802.11ax (Wi-Fi 6) si el modelo de
   ns-3 permite configurarlo de forma reproducible.
2. IEEE 802.15.4 en modo punto a punto. Se denominara "ZigBee-like" porque ns-3
   modela la capa LR-WPAN, no toda la pila ZigBee.
3. LoRa punto a punto, sin asumir una red LoRaWAN.

No se presupone una tecnologia ganadora. Una tecnologia puede resultar inadecuada si
no soporta la carga de telemando; ese resultado tambien es valido.

## Principio de comparacion justa

Se mantienen constantes entre tecnologias:

- geometria, recorrido y velocidad;
- posiciones iniciales del transmisor y receptor;
- carga de aplicacion y duracion;
- definicion de falla y metricas de seguridad;
- nivel ambiental y semilla aleatoria.

Cada tecnologia conserva sus parametros fisicos propios (frecuencia, ancho de banda,
modulacion, sensibilidad y potencia permitida). Esos perfiles deben documentarse y
justificarse por separado. Igualar parametros fisicos incompatibles produciria una
comparacion artificial; lo que se iguala es el servicio de telemando solicitado.

## Piloto minimo

El primer escenario sera un tunel recto sintetico:

- identificador: `G01_straight`;
- longitud: 100 m;
- seccion transversal nominal: 4 m x 4 m;
- sistema de coordenadas: local, en metros;
- transmisor fijo en x = 5 m;
- receptor movil desde x = 10 m;
- velocidad: 2.22 m/s (aproximadamente 8 km/h);
- duracion: 30 s.

La matriz del piloto contiene:

- 1 geometria;
- 3 tecnologias;
- 4 severidades ambientales;
- 3 semillas (`1`, `2`, `3`).

Total esperado: **36 ejecuciones**. Este piloto sirve para comprobar el software, las
trazas y la repetibilidad. Tres semillas no son suficientes para sostener conclusiones
estadisticas finales. La campaña ampliada debera aumentar el numero de geometrías y
semillas despues de validar el piloto.

## Trafico de telemando

Todos los flujos usan UDP. El control incorpora numero de secuencia, marca temporal y
ACK de aplicacion para detectar perdidas sin introducir las retransmisiones y esperas de
TCP.

| Flujo | Carga util | Periodo | Prioridad |
| --- | ---: | ---: | --- |
| Control | 12 bytes | 10 ms | Alta |
| Heartbeat | 4 bytes | 50 ms | Alta |
| Telemetria | 32 bytes | 200 ms | Baja |
| ACK de control | 4 bytes | Por comando valido | Alta |

El tamaño de los paquetes es carga util de aplicacion; los encabezados de las distintas
capas deben registrarse por separado cuando se calcule la ocupacion real del canal.

## Falla de comunicacion y seguridad

Se define una perdida completa de control cuando el receptor no obtiene ningun comando
valido durante 200 ms consecutivos. El estado de la maquina pasa entonces a
`SAFE_STOP`.

Con comandos cada 10 ms, el umbral equivale a 20 comandos consecutivos ausentes. El
evento termina al recuperarse un comando valido; tanto el inicio como el final deben
quedar registrados.

## Interferencia electromagnetica

La frecuencia de red de 50/60 Hz no se modelara como un interferente RF directo porque
no ocupa las bandas de Wi-Fi, IEEE 802.15.4 o LoRa. Los armonicos, variadores, motores,
conmutacion de potencia y ruido impulsivo pueden afectar el receptor, pero su traduccion
a parametros del canal requiere respaldo bibliografico o mediciones.

Durante el piloto se usaran cuatro etiquetas ordinales:

- `ideal`;
- `normal`;
- `degraded`;
- `critical`.

Estas etiquetas no tendran valores numericos inventados. Antes de ejecutar la campaña se
deben calibrar, como minimo, el aumento del ruido efectivo, la probabilidad y duracion de
rafagas interferentes y las perdidas adicionales por obstruccion. Toda perturbacion debe
tener semilla, instante de inicio, duracion y parametros registrados.

## Metricas

Las metricas principales se calculan en la capa de aplicacion del flujo de control:

1. disponibilidad funcional;
2. numero de entradas a `SAFE_STOP`;
3. tiempo acumulado sin control valido;
4. duracion maxima de interrupcion;
5. tiempo de recuperacion del enlace;
6. PDR y PLR de control;
7. latencia en percentiles 95 y 99;
8. jitter en percentil 95;
9. maximo de perdidas consecutivas.

RSSI, SNR, SINR y parametros del PHY se guardaran como variables explicativas cuando el
modelo tecnologico los exponga. No deben sustituir las metricas de seguridad de la capa
de aplicacion.

Objetivos de ingenieria iniciales, sujetos a revision bibliografica:

- PDR de control >= 99 %;
- disponibilidad funcional >= 99.5 %;
- latencia p95 <= 50 ms;
- latencia p99 <= 100 ms;
- jitter p95 <= 20 ms.

## Seleccion de tecnologia

La seleccion no usara por ahora un indice compuesto con pesos arbitrarios. Se aplicara
el siguiente orden de decision:

1. maximizar disponibilidad funcional;
2. minimizar entradas a `SAFE_STOP`;
3. minimizar la interrupcion maxima;
4. maximizar PDR de control;
5. minimizar latencia p99.

Se informaran todos los resultados, incluidos los que no alcancen los objetivos. En caso
de empate se consideraran complejidad de implementacion, energia y compatibilidad con
el futuro controlador ESP32-C3, sin alterar retroactivamente las metricas.

## Reproducibilidad y trazabilidad

Cada fila de resultados debe registrar, como minimo:

- commit de Git;
- version de ns-3;
- identificador de campaña, escenario, tecnologia y severidad;
- semilla y numero de ejecucion;
- parametros completos del PHY, propagacion, movilidad y trafico;
- tiempos de inicio y fin;
- estado de terminacion y errores.

Los resultados crudos pesados permaneceran fuera de Git. Se versionaran configuraciones,
codigo, esquemas, tablas resumen y figuras finales.
