# Metodologia de perfiles PHY para la Fase 1

## Alcance

Este documento fija los perfiles de referencia que se implementaran en ns-3 para
comparar Wi-Fi 6, IEEE 802.15.4 y LoRa punto a punto. Los valores no representan el
maximo teorico de cada tecnologia ni un equipo comercial concreto. Son configuraciones
controladas para un primer experimento reproducible.

El entorno queda fijado en `ns-allinone-3.46.1`. La pagina oficial de la version indica
que ese paquete incluye el modulo contribuido LoRaWAN 0.3.5. Las versiones se registran
en [`environment/versions.yaml`](../environment/versions.yaml).

## Variable de potencia controlada

Los tres perfiles usan 14 dBm de potencia conducida y antenas isotropicas de 0 dBi. Por
tanto, la PIRE nominal simulada es 14 dBm. Esta igualdad evita atribuir al PHY una mejora
que proceda solamente de usar mayor potencia.

El valor no es un limite regulatorio ni garantiza que cualquier radio IEEE 802.15.4 pueda
entregarlo. Es una variable controlada del simulador. Una campaña posterior debera
repetir los casos relevantes con potencias representativas del hardware elegido.

La Resolucion Ministerial 373-2021-MTC recoge limites de PIRE de hasta 36 dBm para
2400-2483.5 MHz y 30 dBm para 915-928 MHz en los casos indicados por la norma. Los
14 dBm del piloto quedan por debajo de ambos valores. Una futura implementacion fisica
seguira necesitando revision de canal, homologacion y condiciones de operacion.

## Unidad comun de comparacion

La unidad comun es el **datagrama de aplicacion**, no UDP. Cada tecnologia recibe la
misma secuencia de cargas utiles, prioridades e instantes de generacion definidos en
[`configs/traffic/telecommand_safety.yaml`](../configs/traffic/telecommand_safety.yaml).

El mapeo sera:

| Tecnologia | Transporte dentro de ns-3 |
| --- | --- |
| Wi-Fi | UDP/IPv4 sobre `WifiNetDevice` |
| IEEE 802.15.4 | Servicio MAC `MCPS-DATA`, sin 6LoWPAN |
| LoRa P2P | Adaptador MAC directo pendiente de implementar |

Esta decision corrige la suposicion inicial de usar UDP para todas las tecnologias. El
codigo de `LoraNetDevice` del modulo 0.3.5 indica expresamente que IP no se usa en
LoRaWAN y no declara el dispositivo como punto a punto. Introducir UDP solo para forzar
una interfaz uniforme agregaria una pila inexistente y sesgaria el resultado.

Cada receptor reconstruira el mismo registro logico de aplicacion: secuencia, tiempo de
generacion, tiempo de recepcion, validez y ACK. Los encabezados, contencion, ACK y
reintentos propios de cada MAC se registraran como costo de la tecnologia.

## Carga ofrecida

La carga maxima de aplicacion, sin encabezados, es:

| Flujo | Calculo | Tasa |
| --- | ---: | ---: |
| Comando de operacion | 10 bytes cada 20 ms | 4000 bit/s |
| ACK / estado minimo | 2 bytes cada 50 ms | 320 bit/s |
| Telemetria basica | 16 bytes cada 500 ms | 256 bit/s |
| **Carga periodica** |  | **4576 bit/s** |
| Reserva para duplicacion o eventos | presupuesto | 424 bit/s |
| **Presupuesto maximo** |  | **5000 bit/s** |

El ACK/estado se usa para observacion y no activa retransmisiones de aplicacion durante
la Fase 1. La reserva no es un flujo periodico ficticio: permite duplicaciones urgentes o
eventos asincronos, cuya activacion debe quedar trazada. Los mecanismos nativos de
reintento MAC permanecen habilitados y sus eventos deben registrarse.

## Perfil Wi-Fi 6

El perfil usa IEEE 802.11ax en 2.4 GHz, canal 1 de 20 MHz, una cadena espacial y
`HeMcs0` fijo. `ConstantRateWifiManager` evita que un algoritmo de adaptacion cambie la
variable que se intenta comparar. Se usa `SpectrumWifiPhy` porque el modelo Yans no
representa interferencia de otras tecnologias; la documentacion de ns-3 exige el modelo
Spectrum para ese caso.

Se deshabilita la agregacion en el piloto para conservar la correspondencia entre un
datagrama y una trama. La topologia consta de un AP y una estacion, con la asociacion
completada antes de iniciar la ventana de medida.

La fuente de `WifiPhy` de ns-3.46.1 fija por defecto una figura de ruido de 7 dB, umbral
de deteccion de -101 dBm a 20 MHz, CCA-ED de -62 dBm y sensibilidad CCA Wi-Fi de
-82 dBm. El perfil los hace explicitos para impedir cambios silenciosos entre versiones.

## Perfil IEEE 802.15.4

El perfil usa la opcion O-QPSK DSSS de 2.4 GHz, pagina 0, canal 11 (2405 MHz), con
250 kbit/s y 62.5 ksimbolos/s. Esos valores coinciden con la tabla implementada por
`LrWpanPhy` y con el PHY IEEE 802.15.4 de referencia.

Se usa CSMA/CA no ranurado, direcciones cortas y confirmacion MAC. No se activa ZigBee
ni 6LoWPAN; por esa razon los resultados se etiquetan **ZigBee-like** y no ZigBee. El
modelo de error de ns-3 representa O-QPSK de 2.4 GHz sobre AWGN y la documentacion
advierte que no constituye una validacion completa contra hardware real.

## Perfil LoRa punto a punto

El perfil mas rapido conservador usa 915.2 MHz, ancho de banda de 125 kHz, SF7,
codificacion 4/5, cabecera explicita, CRC y preambulo de 8 simbolos. La frecuencia se
encuentra dentro de 915-928 MHz y toma AU915-928 como referencia regional para Peru;
no convierte el enlace en LoRaWAN.

La tasa nominal se deriva como:

```text
Rb = SF * BW / 2^SF * 4/5
Rb = 7 * 125000 / 128 * 4/5 = 5468.75 bit/s
```

El presupuesto de 5000 bit/s representa el 91.43 % de la tasa bruta. Sin embargo, esa
comparacion por bits no incluye el costo de formar muchos paquetes pequeños. Con la
formula de tiempo en aire del SX1276, 10 bytes, SF7, BW125, CR 4/5, CRC y cabecera
explicita ocupan aproximadamente 41.216 ms, mientras el periodo de control es 20 ms.
Solo los comandos requeririan el 206.08 % del tiempo disponible. Al añadir ACK/estado y
telemetria, los flujos periodicos requieren como minimo 2.783232 segundos de canal por
segundo, antes de activar la reserva o considerar reintentos.

Por tanto, el perfil LoRa pasa la comprobacion por tasa bruta, pero **falla la prueba previa
global por tiempo en aire**. No se reducira la tasa de generacion ni se descartaran flujos
para mejorar artificialmente el resultado. Una primera simulacion de sobrecarga debe
cuantificar cola, perdidas y entradas a `SAFE_STOP`; despues podra evaluarse un perfil
separado SF7/BW500. Su tiempo en aire estimado es cuatro veces menor y la ocupacion
periodica bajaria a 0.695808, pero debe validarse el costo en sensibilidad, alcance,
regulacion e implementacion antes de sustituir el perfil BW125.

## Limitaciones del modelo

- `SpectrumWifiPhy` opera por paquete y no representa propagacion selectiva en frecuencia
  ni beamforming 802.11ax.
- `LrWpanErrorModel` se basa en O-QPSK sobre AWGN y no sustituye mediciones en tunel.
- El modulo LoRa incluido modela LoRaWAN, no un enlace IP P2P. El adaptador directo es
  un requisito de implementacion y debe probarse con casos unitarios de tiempo en aire.
- Los perfiles de propagacion e interferencia electromagnetica siguen sin calibrar. Estos
  perfiles PHY no inventan sus valores.

## Fuentes tecnicas primarias

- [ns-3.46 y ns-allinone-3.46.1](https://www.nsnam.org/releases/ns-3-46/)
- [Documentacion del modelo Wi-Fi de ns-3](https://www.nsnam.org/docs/release/3.46/models/html/wifi.html)
- [Uso y configuracion de canales Wi-Fi](https://www.nsnam.org/docs/release/3.46/models/html/wifi-user.html)
- [Documentacion LR-WPAN de ns-3](https://www.nsnam.org/docs/release/3.46/models/html/lr-wpan.html)
- [Modulo LoRaWAN del ns-3 App Store](https://apps.nsnam.org/app/lorawan/)
- [Fuente de `LoraNetDevice` 0.3.5](https://github.com/signetlabdei/lorawan/blob/v0.3.5/model/lora-net-device.h)
- [Semtech: LoRa and LoRaWAN Technical Overview](https://www.semtech.com/uploads/technology/LoRa/lora-and-lorawan.pdf)
- [Semtech SX1276](https://www.semtech.com/products/wireless-rf/lora-connect/sx1276)
- [LoRa Alliance RP002](https://resources.lora-alliance.org/home/rp002-1-0-4-regional-parameters)
- [MTC, Resolucion Ministerial 373-2021](https://cdn.www.gob.pe/uploads/document/file/1861732/Resoluci%C3%B3n%20Ministerial%20nro%20373-2021-MTC/01.pdf)
- [MTC, Decreto Supremo 006-2013](https://cdn.www.gob.pe/uploads/document/file/5412657/343700-decreto-supremo-n-006-2013-mtc.pdf)
