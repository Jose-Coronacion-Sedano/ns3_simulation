# Resumen de continuidad — tesis de comunicaciones inalámbricas en túneles con ns-3

**Fecha de corte:** 15 de septiembre de 2026  
**Autor del proyecto:** José Coronación Sedano  
**Repositorio:** <https://github.com/Jose-Coronacion-Sedano/ns3_simulation>  
**Propósito de este documento:** permitir retomar el trabajo en otra computadora sin depender del historial de conversación.

---

## 1. Resumen ejecutivo

La tesis estudia enlaces inalámbricos punto a punto para telemando y seguridad en túneles mineros, mediante simulaciones reproducibles en ns-3. El objetivo inmediato es comparar tecnologías bajo la misma geometría, movilidad, potencia y servicio de aplicación, registrar métricas de confiabilidad y preparar posteriormente un conjunto de datos para selección/adaptación de enlace mediante una red neuronal ejecutable en ESP32-C3/TinyML.

La orientación técnica evolucionó durante la conversación. La decisión más reciente es concentrar la comparación en **tres tecnologías que operen alrededor de 915 MHz y usarlas solamente para control**, no para video:

1. **LoRa punto a punto:** SF7, BW 250 kHz, alrededor de 915.2 MHz.
2. **IEEE 802.15.4g MR-FSK / 2-GFSK:** 915 MHz.
3. **Wi-Fi HaLow IEEE 802.11ah:** canal de 1 o 2 MHz alrededor de 915 MHz.

El tráfico de aplicación más reciente es:

- comando de control: **8 bytes**;
- ACK de aplicación separado: **1 byte**;
- telemetría mínima: **1 byte**;
- periodo normal de comando: **50 ms** para LoRa y HaLow;
- periodo solicitado para MR-FSK: **10 ms**;
- periodo de telemetría heredado de la última configuración: **150 ms**;
- umbral recomendado de pérdida completa de control: **SAFE_STOP después de 200 ms sin recibir un comando válido**.

Es fundamental advertir que estas decisiones nuevas **todavía no están incorporadas en el repositorio**. El commit actual sigue describiendo Wi-Fi 6 a 2.4 GHz, IEEE 802.15.4 O-QPSK a 2.4 GHz, LoRa SF7/BW250 y telemetría de 8 bytes. Por ello, el primer trabajo en la nueva computadora debe ser actualizar de forma trazable la especificación y los YAML antes de implementar código.

---

## 2. Estado de Git y GitHub al momento del relevo

### 2.1 Copia local

Directorio local usado durante el trabajo:

```text
C:\Users\jcoronacion\Documents\Codex\2026-08-11\github-plugin-github-openai-curated-remote-2\work\ns3_simulation
```

Remoto configurado:

```text
origin  https://github.com/Jose-Coronacion-Sedano/ns3_simulation.git
```

Rama activa:

```text
agent/phase1-initial-comparison
```

Commit activo y último commit remoto conocido:

```text
361281e Adopt SF7/BW250 traffic schedule
```

La salida local indicaba que `HEAD` y `origin/agent/phase1-initial-comparison` apuntaban al mismo commit y que no había cambios sin confirmar.

Historial relevante:

```text
361281e Adopt SF7/BW250 traffic schedule
3b05df6 Adopt common 5 kbit/s traffic profile
340f9c6 Define Phase 1 PHY reference profiles
3b0ce24 Define Phase 1 comparison experiment
30cb37d Create thesis project folder architecture
fede78d Initial thesis project architecture
```

La rama `main` local y el último `origin/main` conocido apuntaban a:

```text
30cb37d Create thesis project folder architecture
```

### 2.2 Pull request

Durante el trabajo anterior existía el **pull request 1**, presumiblemente:

<https://github.com/Jose-Coronacion-Sedano/ns3_simulation/pull/1>

No se pudo verificar su estado final el 15 de septiembre de 2026 porque GitHub CLI respondió `HTTP 401`.

### 2.3 Estado actual de autenticación

Anteriormente GitHub CLI estuvo autenticado como `Jose-Coronacion-Sedano`, con protocolo HTTPS y permisos de repositorio. En la última verificación, la credencial almacenada ya no era válida:

```text
X Failed to log in to github.com account Jose-Coronacion-Sedano
The token in default is invalid.
HTTP 401: Requires authentication
```

También falló `git ls-remote` por falta de credenciales HTTPS. No se expuso ni se debe copiar ningún token.

En la nueva computadora se debe realizar una autenticación nueva:

```powershell
gh auth login -h github.com
gh auth status
```

Seleccionar GitHub.com, HTTPS y autenticación mediante navegador. Las credenciales del llavero de una computadora no se deben transferir manualmente a otra.

---

## 3. Archivos externos que no están garantizados en GitHub

El plan original de tesis se encontraba en:

```text
C:\Users\jcoronacion\Downloads\plan_tesis-jcoronacion1-2025.pdf
```

También se recibió un texto adjunto relacionado con la Fase 1 en una carpeta interna de Codex. Esos archivos no forman parte necesariamente del repositorio. Deben copiarse de manera segura a la nueva computadora o incorporarse al repositorio únicamente si los derechos, el tamaño y la privacidad lo permiten.

Del plan de tesis se extrajeron los siguientes puntos:

- **OE1:** simular enlaces inalámbricos punto a punto en condiciones subterráneas/mineras.
- **OE2:** registrar y analizar el desempeño del enlace para preparar el conjunto de datos.
- **OE3:** desarrollar posteriormente selección/adaptación mediante red neuronal y ESP32/TinyML.
- **OE4:** evaluar posteriormente estrategias de redundancia.
- Las geometrías previstas incluyen túneles rectos, curvas, intersecciones y obstrucciones, con distancias aproximadas entre 10 y 100 m.
- Las métricas físicas previstas incluyen pérdida de trayectoria, SINR y cobertura; durante la conversación se añadieron métricas funcionales de seguridad.

La Fase 1 debe cubrir principalmente OE1 y OE2. Red neuronal, TinyML, firmware y redundancia pertenecen a fases posteriores.

---

## 4. Evolución de las decisiones de tráfico

### 4.1 Primera propuesta

La arquitectura inicial consideraba:

| Señal | Payload | Periodo inicial | Tasa aproximada |
|---|---:|---:|---:|
| Comando | 10 B | 20 ms | 4.0 kbit/s |
| ACK/estado | 2 B | 50 ms | 0.32 kbit/s |
| Telemetría | 16 B | 500 ms | 0.256 kbit/s |
| Reserva | — | — | 0.424 kbit/s |

### 4.2 Cambios discutidos

Se exploraron sucesivamente:

- reducir comando de 10 a 8 bytes;
- representar 7 bytes de mando más 1 byte asociado a ACK/estado;
- separar finalmente el ACK en su propio paquete;
- cambiar el periodo de comando de 20 a 25 ms y luego a 50 ms;
- reducir la telemetría de 16 a 8 bytes;
- cambiar el periodo de telemetría de 500 a 150 ms;
- usar LoRa SF7/BW250 en lugar de SF7/BW125;
- mantener SAFE_STOP inicialmente en 200 ms;
- analizar 250 ms como sensibilidad, sin adoptarlo definitivamente;
- reducir finalmente la telemetría mínima a 1 byte para las tres tecnologías nuevas;
- solicitar un periodo de comando de 10 ms específicamente para MR-FSK.

### 4.3 Perfil más reciente asumido

Se interpretó que el ACK de 1 byte se genera por cada comando válido y utiliza el mismo periodo nominal que el comando de la tecnología.

| Tecnología | Señal | Payload | Periodo | Frecuencia | Tasa útil |
|---|---|---:|---:|---:|---:|
| LoRa SF7/BW250 | Comando | 8 B | 50 ms | 20 Hz | 1280 bit/s |
| LoRa SF7/BW250 | ACK separado | 1 B | 50 ms | 20 Hz | 160 bit/s |
| LoRa SF7/BW250 | Telemetría | 1 B | 150 ms | 6.667 Hz | 53.333 bit/s |
| **Total LoRa** | | | | | **1493.333 bit/s** |
| MR-FSK | Comando | 8 B | 10 ms | 100 Hz | 6400 bit/s |
| MR-FSK | ACK separado | 1 B | 10 ms | 100 Hz | 800 bit/s |
| MR-FSK | Telemetría | 1 B | 150 ms | 6.667 Hz | 53.333 bit/s |
| **Total MR-FSK** | | | | | **7253.333 bit/s** |
| Wi-Fi HaLow | Comando | 8 B | 50 ms | 20 Hz | 1280 bit/s |
| Wi-Fi HaLow | ACK separado | 1 B | 50 ms | 20 Hz | 160 bit/s |
| Wi-Fi HaLow | Telemetría | 1 B | 150 ms | 6.667 Hz | 53.333 bit/s |
| **Total HaLow** | | | | | **1493.333 bit/s** |

Estas son tasas de payload de aplicación. No incluyen preámbulo, cabeceras PHY/MAC, CRC, separación entre tramas, acceso al medio, ACK nativos de la MAC ni retransmisiones.

### 4.4 Corrección terminológica

Decir “disminuir la frecuencia de transmisión a 10 ms” es impreciso. Diez milisegundos es un **periodo**; disminuir el periodo de 50 a 10 ms significa **aumentar la frecuencia** de transmisión de 20 a 100 Hz.

### 4.5 Contenido del byte de telemetría

Un byte solo puede representar telemetría mínima, por ejemplo ocho banderas, un estado enumerado o una magnitud cuantizada. No puede sustituir una trama multivariable de sensores sin definir codificación, escalas y pérdida de resolución. Antes de implementar se debe definir un mapa de bits reproducible.

---

## 5. Comparación justa y escenario optimizado

Existe una diferencia metodológica importante: si MR-FSK utiliza 10 ms mientras LoRa y HaLow usan 50 ms, las tecnologías ya no reciben la misma carga.

Se recomendó separar dos experimentos.

### 5.1 Experimento A — comparación común

Las tres tecnologías usan:

- comando de 8 B cada 50 ms;
- ACK de 1 B por comando, nominalmente cada 50 ms;
- telemetría de 1 B cada 150 ms.

Carga programada común:

```text
1493.333 bit/s
```

Este experimento permite atribuir diferencias a la tecnología y al canal, no a cargas distintas.

### 5.2 Experimento B — configuración optimizada de MR-FSK

- LoRa: 50 ms.
- HaLow: 50 ms.
- MR-FSK: 10 ms.
- ACK con el mismo periodo de cada comando.
- Telemetría de 1 B cada 150 ms para todas.

Este experimento mide la ventaja de reacción temporal de MR-FSK, pero debe denominarse “configuración optimizada” o “escenario específico por tecnología”, no “comparación bajo idénticas condiciones”.

### 5.3 Tamaño recomendado del piloto

El piloto existente tiene 3 tecnologías × 4 severidades × 3 semillas = 36 ejecuciones.

Si se conserva una comparación común y se agrega MR-FSK optimizado:

- comparación común: 36 ejecuciones;
- MR-FSK a 10 ms: 1 tecnología × 4 severidades × 3 semillas = 12 ejecuciones;
- total recomendado del piloto ampliado: **48 ejecuciones**.

Tres semillas sirven para depuración y repetibilidad básica, no para conclusiones estadísticas finales. Después del piloto debe aumentarse el número de semillas.

---

## 6. SAFE_STOP y reacción del sistema

Se recomendó mantener un umbral temporal absoluto:

```text
SAFE_STOP = 200 ms sin recibir un comando válido
```

Equivalencias:

| Tecnología/configuración | Periodo | Pérdidas consecutivas equivalentes a 200 ms |
|---|---:|---:|
| LoRa | 50 ms | 4 |
| HaLow | 50 ms | 4 |
| MR-FSK optimizado | 10 ms | 20 |

Para un SAFE_STOP de 250 ms las equivalencias serían 5, 5 y 25, respectivamente.

No se debe usar un número fijo de cuatro paquetes para todas las tecnologías: en MR-FSK a 10 ms provocaría parada después de solo 40 ms. La implementación debe comparar marcas temporales, no únicamente contar paquetes.

El argumento humano de que una respuesta por debajo de unos 100 ms suele sentirse inmediata puede justificar un periodo de mando de 50 ms, pero no debe confundirse con un requisito de seguridad funcional. El tiempo total de reacción incluye generación, espera de acceso al canal, tiempo en aire, procesamiento, actuación y eventual confirmación.

---

## 7. Evaluación LoRa SF7/BW250 con el tráfico nuevo

Parámetros de referencia:

- frecuencia: 915.2 MHz;
- SF7;
- BW 250 kHz;
- CR 4/5;
- preámbulo de 8 símbolos;
- cabecera explícita;
- CRC activado;
- enlace directo half-duplex;
- no LoRaWAN.

Tiempos en aire calculados:

| Paquete | Payload | Tiempo en aire aproximado |
|---|---:|---:|
| Comando | 8 B | 18.048 ms |
| ACK | 1 B | 12.928 ms |
| Telemetría | 1 B | 12.928 ms |

Ocupación media programada:

```text
Comandos:    20/s × 18.048 ms = 360.960 ms/s
ACK:         20/s × 12.928 ms = 258.560 ms/s
Telemetría:  6.667/s × 12.928 ms =  86.187 ms/s
Total:                           ≈ 705.707 ms/s
```

Ocupación ideal aproximada:

```text
70.57 %
```

Cuando coinciden comando, ACK y telemetría:

```text
18.048 + 12.928 + 12.928 = 43.904 ms
```

Quedan aproximadamente 6.096 ms de un ciclo de 50 ms. Esto es más holgado que la configuración anterior con telemetría de 8 B, pero todavía no permite asumir que siempre cabrá una retransmisión completa.

La telemetría debe conservar:

- prioridad baja;
- desplazamiento si hay comando o ACK pendiente;
- política `latest_value_wins`;
- registro del instante de generación, transmisión, reemplazo y recepción.

No se recomienda utilizar periodos de 10 ms para LoRa SF7/BW250: el comando de 8 B por sí solo ocupa alrededor de 18 ms.

### 7.1 Reserva de 424 bit/s

El repositorio actual conserva una reserva de seguridad de 424 bit/s. La última decisión del usuario no indicó expresamente si debe eliminarse. Por tanto, queda pendiente.

Si se conserva como presupuesto y se usa completamente para duplicar comandos de 8 B, agrega aproximadamente 119.568 ms/s de tiempo en aire LoRa. Con la telemetría nueva de 1 B, la ocupación ideal máxima subiría aproximadamente a:

```text
70.57 % + 11.96 % = 82.53 %
```

La reserva no debe modelarse como flujo constante. Debe activarse únicamente para comandos urgentes duplicados o eventos asíncronos, y cada activación debe registrarse.

### 7.2 Limitación del modelo LoRa

El módulo LoRa disponible utiliza tablas de sensibilidad documentadas principalmente para BW125. Aunque acepta BW250 para calcular tiempo en aire, no se debe reutilizar silenciosamente la sensibilidad de BW125. Debe calibrarse una sensibilidad BW250 o incorporarse una curva PER/SNR apropiada.

---

## 8. Evaluación IEEE 802.15.4g MR-FSK a 10 ms

Con comando y ACK cada 10 ms, la carga de aplicación es 7253.333 bit/s. La utilización mínima respecto de diferentes tasas PHY sería:

| Tasa PHY MR-FSK | Utilización mínima por payload |
|---:|---:|
| 50 kbit/s | 14.51 % |
| 100 kbit/s | 7.25 % |
| 200 kbit/s | 3.63 % |
| 400 kbit/s | 1.81 % |

El enlace genera aproximadamente:

- 100 comandos/s;
- hasta 100 ACK de aplicación/s;
- 6.667 telemetrías/s;
- alrededor de 207 datagramas de aplicación/s.

El periodo de 10 ms parece viable por capacidad, pero se debe comprobar el ciclo completo con preámbulo, SFD, PHR, cabeceras MAC, FCS, CCA/CSMA, tiempos de turnaround, ACK MAC y reintentos.

Recomendación inicial:

- tasa principal MR-FSK: **100 o 200 kbit/s**;
- 50 kbit/s: escenario de máxima robustez y sensibilidad;
- 400 kbit/s: escenario de máxima capacidad;
- potencia común del piloto: 14 dBm conducidos y 0 dBi de antena, salvo que se redefina la variable controlada.

### 8.1 Parámetros MR-FSK aún sin decidir

“2-GFSK a 915 MHz” no es una especificación reproducible suficiente. Antes de implementar deben fijarse:

- tasa exacta: 50/100/200/400 kbit/s;
- desviación de frecuencia;
- índice de modulación;
- ancho de canal y máscara espectral;
- preámbulo y SFD;
- FEC activado o desactivado;
- whitening;
- longitud de trama y cabeceras;
- CCA/CSMA o acceso programado;
- sensibilidad y curva PER frente a SNR;
- ACK nativo MAC y política de reintentos;
- equipo de referencia, por ejemplo CC1312R, sin confundir sus modos propietarios con MR-FSK estandarizado.

IEEE 802.15.4g MR-FSK puede manejar holgadamente el telemando, pero no video H.265 de 1.5–2 Mbit/s. Esa conclusión fue una de las razones para restringir la nueva comparación solamente a control.

---

## 9. Evaluación Wi-Fi HaLow para control

HaLow a 1 o 2 MHz posee capacidad muy superior a la carga de 1493.333 bit/s. Para control se recomendó priorizar robustez, no velocidad:

- 915 MHz;
- 1 MHz como perfil principal;
- MCS0 o MCS10 como modos robustos, sujeto a la implementación elegida;
- 2 MHz como caso de sensibilidad/capacidad;
- ACK de aplicación de 1 B para conservar la comparación lógica;
- registrar también ACK y reintentos nativos Wi-Fi;
- sin agregación durante el piloto, si se desea conservar correspondencia entre datagrama y trama.

HaLow tiene más overhead y consumo de activación que un radio Sub-GHz simple para paquetes de 8 B, pero ofrece OFDM, adaptación de tasa y capacidad sobrante. Como ya no se exige video, la justificación debe centrarse en robustez, latencia y posibilidad futura de integrar datos de mayor tasa.

---

## 10. Por qué se eligió 915 MHz para los túneles

La conclusión adoptada es que 915 MHz constituye un **muy buen compromiso práctico** para control subterráneo, pero no debe escribirse que es universalmente “la mejor frecuencia”.

Ventajas:

- aproximadamente 8.37 dB menos pérdida en espacio libre que 2.4 GHz para igual distancia y ganancias;
- mejor difracción alrededor de curvas que 2.4/5.8 GHz;
- menor efecto relativo de rugosidades y obstáculos respecto de la longitud de onda;
- antena de cuarto de onda manejable, alrededor de 8.2 cm;
- disponibilidad de radios industriales LoRa, FSK y HaLow;
- posibilidad de comparar diferentes PHY dentro de la misma banda.

Matiz esencial: en túneles rectos, el comportamiento como guía de onda puede producir pendientes de atenuación menores a frecuencias más altas en la zona lejana. En curvas de 90 grados, mediciones de NIOSH muestran que 455 y 915 MHz tienden a sufrir una caída menos precipitada que 2.45 y 5.8 GHz. El resultado depende de sección transversal, polarización, rugosidad, distancia, conductores metálicos y posición de las antenas.

Redacción académica recomendada:

> Se seleccionó la banda Sub-GHz de 915 MHz porque representa un compromiso favorable entre pérdida de propagación, difracción, dimensiones de antena y disponibilidad de dispositivos industriales. Diversas campañas de medición en minas y túneles han reportado un comportamiento robusto de esta banda en escenarios con línea de vista y una menor caída relativa alrededor de curvas respecto de frecuencias de 2.45 y 5.8 GHz. Sin embargo, no se considera que 915 MHz sea universalmente la frecuencia de menor atenuación, puesto que el efecto de guía de onda puede favorecer frecuencias superiores en galerías rectas. En consecuencia, la evaluación distinguirá explícitamente entre tramos rectos, curvas, intersecciones y obstrucciones.

---

## 11. Soporte real de las tecnologías en ns-3

Este es uno de los mayores riesgos del proyecto.

### 11.1 Versión actualmente registrada

El repositorio fija:

```text
ns-allinone-3.46.1
lorawan 0.3.5 incluido en el paquete
```

Se inspeccionó localmente LoRaWAN 0.3.7, pero no se adoptó ni se registró un cambio de versión. No se recomendó migrar a ns-3.48 únicamente por el número de versión: cualquier migración requiere pruebas de compilación, trazas, tiempos en aire y compatibilidad del módulo.

### 11.2 LoRa P2P

El módulo incluido modela LoRaWAN, no un enlace IP punto a punto. `LoraNetDevice` no debe forzarse artificialmente a transportar UDP/IP. Se requiere un adaptador MAC directo que preserve los mismos payloads y marcas temporales de la aplicación.

### 11.3 IEEE 802.15.4g MR-FSK

El módulo LR-WPAN principal de ns-3 implementa como referencia el PHY O-QPSK de 2.4 GHz a 250 kbit/s. No implementa MR-FSK de 802.15.4g. No basta con cambiar la tasa o la frecuencia del YAML: se requieren modelo PHY, PSD, temporización, error, sensibilidad, interferencia y posiblemente cambios MAC.

### 11.4 Wi-Fi HaLow

El módulo Wi-Fi principal de ns-3 no implementa IEEE 802.11ah. Existe un módulo académico antiguo de imec basado principalmente en ns-3.23/ns-3.25 y marcado como alfa. Portarlo a ns-3 moderno implica adaptar APIs Wi-Fi, compilación, modelos de error y validar resultados.

### 11.5 Opciones de implementación

Antes de programar debe elegirse explícitamente una estrategia:

1. **Modelos estándar completos:** portar/desarrollar MR-FSK y HaLow. Máxima fidelidad, esfuerzo muy alto.
2. **PHY abstracto calibrado:** reproducir tiempo en aire, sensibilidad, curvas PER/SNR, acceso al canal y estados del enlace sin implementar cada señal. Es más manejable para la tesis, pero debe etiquetarse como modelo abstracto y validarse.
3. **Mantener tecnologías ya soportadas:** conservar Wi-Fi 6 y 802.15.4 O-QPSK. Menor riesgo de software, pero contradice la decisión más reciente de comparar únicamente tecnologías de 915 MHz.

La recomendación práctica es evaluar seriamente la opción 2 para la Fase 1, usando curvas y parámetros publicados o medidos, y reservar una implementación completa como contribución futura. No se deben presentar resultados de un PHY genérico como si fueran resultados estandarizados de 802.11ah o 802.15.4g.

---

## 12. Modelo de propagación para el túnel

No se debe afirmar que `LogDistance + Rayleigh` representa fielmente un túnel. Es una línea base estadística, no un modelo completo de guía de onda, curvas y multipath.

### 12.1 Arquitectura recomendada para Fase 1

```text
MultiModelSpectrumChannel
 ├─ ConstantSpeedPropagationDelayModel
 ├─ TunnelTwoSlopePropagationLossModel       [personalizado]
 ├─ CorrelatedLognormalShadowingModel        [personalizado]
 └─ TunnelStateAwareFadingModel              [Rician/Nakagami]
```

Como primera aproximación ejecutable se puede utilizar:

```text
ThreeLogDistancePropagationLossModel
  + NakagamiPropagationLossModel
```

con parámetros diferentes y justificados para túnel recto, curva, intersección y obstrucción.

No deben apilarse `LogDistance` y `ThreeLogDistance` para representar la misma pérdida de gran escala, ni Rayleigh y Nakagami con `m=1` simultáneamente.

### 12.2 Pérdida de gran escala

Modelo conceptual:

```text
PL(d) = PL(d0) + 10*n_i*log10(d/d0)
        + L_curva + L_obstáculo + X_sigma
```

donde:

- `n_i` puede cambiar entre zona cercana, zona de guía y zona lejana;
- `L_curva` representa pérdida adicional en una curva;
- `L_obstáculo` representa vehículo, maquinaria o persona;
- `X_sigma` representa shadowing lognormal correlacionado espacialmente.

Los estados mínimos recomendados son:

- `LOS_STRAIGHT`;
- `NLOS_BEND`;
- `JUNCTION`;
- `PARTIAL_BLOCKAGE`;
- `HEAVY_BLOCKAGE`.

### 12.3 Fading

Punto de partida:

| Estado | Modelo inicial |
|---|---|
| Túnel recto LoS | Rician o Nakagami con `m > 1` |
| Curva/NLoS | Nakagami con `m ≈ 1` |
| Obstrucción severa | Nakagami con `m < 1`, solo con respaldo |

Los valores deben calibrarse; no deben inventarse.

### 12.4 Correlación temporal

Para velocidad de 2.22 m/s a 915 MHz:

```text
longitud de onda ≈ 0.328 m
Doppler máximo ≈ 6.8 Hz
tiempo de coherencia aproximado ≈ 62 ms
```

Este tiempo es del mismo orden que el periodo normal de comando de 50 ms. Un fading independiente por paquete puede distorsionar las ráfagas de pérdida y el número de SAFE_STOP. Se necesita correlación temporal/espacial.

### 12.5 Multipath real

Para comparar OFDM HaLow contra FSK y LoRa con rigor se necesita, idealmente, una respuesta impulsional o una respuesta en frecuencia dependiente de posición. Puede derivarse de mediciones, ray tracing, RT-FDTD o un modelo de taps. Un `SpectrumPropagationLossModel` personalizado podría aplicar la respuesta por subbanda, pero el error de paquete de cada PHY también debe interpretar correctamente el SINR.

HaLow OFDM debería resistir mejor un canal selectivo en frecuencia, mientras que LoRa/FSK pueden conservar un enlace de baja tasa a SNR menores. Esto no implica que HaLow siempre tenga mayor alcance alrededor de curvas.

---

## 13. Interferencia electromagnética

La red eléctrica de 50/60 Hz no debe añadirse directamente como interferencia RF en 915 MHz. Los efectos relevantes son:

- armónicos y emisiones conducidas/radiadas;
- variadores de frecuencia;
- motores y contactores;
- convertidores conmutados;
- arcos eléctricos;
- ruido impulsivo;
- interferentes estrechos o de banda ancha.

Estos fenómenos deben modelarse de forma separada de la propagación:

- PSD del interferente;
- frecuencia central y ancho de banda;
- potencia recibida;
- ciclo ON/OFF;
- probabilidad, duración y repetición de ráfagas;
- ubicación de la fuente;
- semilla aleatoria.

El repositorio usa actualmente cuatro etiquetas sin parámetros inventados:

- `ideal`;
- `normal`;
- `degraded`;
- `critical`.

Antes de ejecutar resultados científicos deben asignarse valores mediante literatura o mediciones. Hasta entonces la campaña debe permanecer marcada como no ejecutable.

---

## 14. Escenario piloto registrado actualmente

El YAML existente define:

- identificador: `G01_straight`;
- túnel recto sintético;
- longitud: 100 m;
- ancho: 4 m;
- alto: 4 m;
- transmisor fijo en `x=5 m`, `z=2 m`;
- receptor móvil desde `x=10 m`, `z=2 m`;
- velocidad: 2.22 m/s, aproximadamente 8 km/h;
- duración: 30 s;
- severidades: 4;
- semillas: 1, 2 y 3;
- 36 ejecuciones en el diseño actual.

Después del piloto deben añadirse geometrías que representen:

1. túnel recto;
2. curva de 90 grados;
3. intersección/T;
4. vehículo o maquinaria bloqueando parcialmente;
5. obstrucción crítica o tramo NLoS.

---

## 15. Métricas y criterio de selección

Métricas funcionales principales ya definidas:

1. disponibilidad funcional;
2. cantidad de entradas a SAFE_STOP;
3. tiempo acumulado sin control válido;
4. duración máxima de interrupción;
5. tiempo de recuperación;
6. PDR y PLR de control;
7. latencia p95 y p99;
8. jitter p95;
9. máximo de pérdidas consecutivas.

Variables explicativas:

- potencia recibida/RSSI;
- SNR;
- SINR;
- MCS/modo PHY;
- reintentos MAC;
- ocupación y tiempo en aire;
- estado del túnel;
- potencia de interferencia;
- velocidad y posición.

Objetivos iniciales registrados, todavía sujetos a respaldo bibliográfico:

- PDR de control ≥ 99 %;
- disponibilidad funcional ≥ 99.5 %;
- latencia p95 ≤ 50 ms;
- latencia p99 ≤ 100 ms;
- jitter p95 ≤ 20 ms.

La selección debe seguir un orden lexicográfico, no un índice ponderado arbitrario:

1. maximizar disponibilidad;
2. minimizar entradas a SAFE_STOP;
3. minimizar interrupción máxima;
4. maximizar PDR;
5. minimizar latencia p99.

Se debe informar también cuando una tecnología no satisface el servicio.

---

## 16. Estado efectivo del repositorio

### 16.1 Lo que sí está registrado

- estructura general de tesis, simulación, ML, firmware y visualización;
- metodología inicial;
- esquema de métricas y dataset;
- piloto de 36 ejecuciones;
- perfil de tráfico con comando 8 B/50 ms, ACK 1 B/50 ms, telemetría 8 B/150 ms y reserva 424 bit/s;
- perfiles Wi-Fi 6, IEEE 802.15.4 O-QPSK y LoRa SF7/BW250;
- versión ns-3.46.1 y LoRaWAN 0.3.5;
- archivos de configuración y documentación;
- placeholders para aplicaciones, instaladores, propagación, métricas, seguridad y geometría.

### 16.2 Lo que no está implementado

Los archivos C++ principales inspeccionados, incluidos los escenarios `01`, `02`, `03` y el modelo de propagación del túnel, están vacíos. La campaña se declara expresamente:

```text
executable: false
```

Bloqueadores registrados:

- calibrar niveles de interferencia;
- implementar instaladores de tecnologías;
- implementar adaptador LoRa P2P;
- calibrar sensibilidad LoRa BW250;
- implementar escenario;
- implementar recolector de métricas.

### 16.3 Diferencias entre GitHub y las decisiones nuevas

| Tema | Repositorio actual | Decisión reciente |
|---|---|---|
| Tecnología 1 | Wi-Fi 6, 2.4 GHz | HaLow, 915 MHz |
| Tecnología 2 | 802.15.4 O-QPSK, 2.4 GHz | 802.15.4g MR-FSK, 915 MHz |
| Tecnología 3 | LoRa P2P 915.2 MHz | Se mantiene |
| Telemetría | 8 B/150 ms | 1 B/150 ms |
| MR-FSK | No existe | 10 ms optimizado; 50 ms común recomendado |
| SAFE_STOP | 200 ms/4 pérdidas a 50 ms | 200 ms temporal; 20 pérdidas a 10 ms |
| Piloto | 36 ejecuciones | 48 recomendadas si se agregan ambos perfiles |
| Video | Fuera del repositorio | Definitivamente fuera de esta comparación de control |

---

## 17. Archivos que deben modificarse primero

Sin escribir todavía C++, actualizar en este orden:

1. `README.md`: reemplazar alcance tecnológico y explicar comparación común/optimizada.
2. `docs/experimental_design.md`: registrar las tres tecnologías de 915 MHz y el diseño de 48 ejecuciones.
3. `docs/methodology.md`: describir modelos abstractos o puertos elegidos y sus limitaciones.
4. `docs/architecture.md`: definir plano de aplicación común y adaptadores por tecnología.
5. `configs/traffic/telecommand_safety.yaml`:
   - telemetría de 8 a 1 B;
   - mantener 150 ms;
   - declarar perfiles `common_50ms` y `mrfsk_optimized_10ms`, o crear dos YAML;
   - recalcular tasas;
   - definir el estado de la reserva de 424 bit/s.
6. Crear `configs/technologies/ieee802154g_mrfsk_reference.yaml`.
7. Crear `configs/technologies/wifi_halow_reference.yaml`.
8. Actualizar `configs/technologies/lora_p2p_reference.yaml` para telemetría de 1 B y ocupación de 70.57 %.
9. Actualizar `configs/campaigns/C01_initial_comparison.yaml`.
10. Actualizar `configs/scenarios/compare_technologies.yaml`.
11. Actualizar esquemas de resultados para guardar modo PHY, tiempo en aire y tipo de modelo real/abstracto.

Cada cambio conceptual debe hacerse primero en documentación/configuración y luego en C++.

---

## 18. Plan de implementación recomendado

### Paso 0 — recuperar acceso y confirmar Git

- instalar Git y GitHub CLI;
- autenticarse;
- clonar;
- comprobar rama y commit;
- revisar pull request 1;
- no mezclar cambios con `main` sin revisar el PR.

### Paso 1 — consolidar especificación

- decidir si la reserva de 424 bit/s continúa;
- confirmar que ACK se genera por cada comando;
- definir el byte de telemetría;
- fijar MR-FSK primario a 100 o 200 kbit/s;
- fijar HaLow primario a 1 MHz y un MCS robusto;
- aprobar experimento común más experimento optimizado.

### Paso 2 — elegir nivel de fidelidad

- decidir entre módulos completos o PHY abstractos calibrados;
- documentar cualquier simplificación;
- no etiquetar un PHY genérico como estándar completo.

### Paso 3 — implementar servicio común

- generador de comandos;
- ACK de aplicación separado;
- telemetría desplazable;
- `latest_value_wins`;
- marcas temporales y números de secuencia;
- SAFE_STOP basado en tiempo.

### Paso 4 — canal básico de túnel

- geometría recta;
- pérdida two/three-slope calibrable;
- shadowing correlacionado;
- fading según estado;
- pruebas unitarias deterministas.

### Paso 5 — adaptadores PHY/MAC

- LoRa P2P directo;
- MR-FSK abstracto o completo;
- HaLow portado o abstracto;
- misma API lógica de aplicación;
- registrar overhead y eventos nativos.

### Paso 6 — métricas y pruebas

- comprobar PDR sin pérdidas en canal ideal;
- comprobar tiempos en aire;
- comprobar que telemetría nunca retrasa control;
- provocar cuatro/veinte pérdidas y verificar SAFE_STOP a 200 ms;
- comprobar determinismo de semillas;
- validar archivos CSV contra schemas.

### Paso 7 — piloto

- ejecutar primero una semilla y un escenario ideal;
- después completar las 48 ejecuciones propuestas;
- no obtener conclusiones finales con tres semillas;
- ampliar semillas y geometrías tras validar software.

---

## 19. Comandos para continuar en otra computadora

En PowerShell:

```powershell
winget install --id Git.Git -e
winget install --id GitHub.cli -e
```

Cerrar y volver a abrir PowerShell para refrescar `PATH`.

```powershell
gh auth login -h github.com
gh auth status
```

Clonar y seleccionar la rama:

```powershell
cd <DIRECTORIO_DE_TRABAJO>
gh repo clone Jose-Coronacion-Sedano/ns3_simulation
cd ns3_simulation
git fetch --all --prune
git switch agent/phase1-initial-comparison
git status
git rev-parse HEAD
```

El hash esperado antes de incorporar decisiones nuevas es:

```text
361281e
```

Comprobar el PR:

```powershell
gh pr view 1
gh pr status
```

Para ns-3 se recomienda un entorno Linux o WSL2, porque es el camino más habitual y reproducible para compilación. El repositorio actual fija ns-allinone-3.46.1; no cambiar a ns-3.48 ni LoRaWAN 0.3.7 hasta que exista una prueba de compatibilidad y se actualice `environment/versions.yaml`.

---

## 20. Decisiones confirmadas, supuestos y pendientes

### Confirmadas por la conversación

- trabajar paso a paso y mantener trazabilidad en GitHub;
- concentrar el estudio nuevo en control;
- tecnologías objetivo: LoRa P2P, 802.15.4g MR-FSK y Wi-Fi HaLow;
- banda objetivo: 915 MHz;
- comando: 8 B;
- ACK separado: 1 B;
- telemetría mínima: 1 B;
- periodo solicitado MR-FSK: 10 ms;
- LoRa de referencia: SF7/BW250;
- telemetría desplazable y de baja prioridad;
- comparar desempeño en túneles.

### Supuestos usados y que conviene ratificar

- LoRa y HaLow conservan periodo de comando de 50 ms;
- el ACK de aplicación se genera por cada comando válido;
- el ACK tiene el mismo periodo nominal que el comando;
- la telemetría mantiene 150 ms en las tres tecnologías;
- SAFE_STOP continúa en 200 ms;
- HaLow usa 1 MHz como perfil principal y 2 MHz como sensibilidad;
- MR-FSK usa 100 o 200 kbit/s como perfil principal.

### Pendientes que cambian resultados

1. ¿Se conserva la reserva de 424 bit/s?
2. ¿Qué bits contiene exactamente el byte de telemetría?
3. ¿Se usará ACK de aplicación además del ACK nativo de cada MAC?
4. ¿Cuál será la tasa y configuración exacta de MR-FSK?
5. ¿Cuál será el MCS y ancho primario de HaLow?
6. ¿Se implementarán PHY completos o abstracciones calibradas?
7. ¿Se mantienen dos experimentos —común y optimizado—?
8. ¿Se conserva ns-3.46.1 o se abre una rama separada para migración?
9. ¿Qué parámetros medidos o bibliográficos definirán los cuatro niveles de interferencia?

No deben ejecutarse campañas ni presentarse resultados antes de cerrar por lo menos los puntos 3–7.

---

## 21. Referencias técnicas utilizadas

- ns-3 Wi-Fi: <https://www.nsnam.org/docs/models/html/wifi-user.html>
- ns-3 LR-WPAN: <https://www.nsnam.org/docs/models/singlehtml/>
- ns-3 propagation: <https://www.nsnam.org/docs/models/html/propagation.html>
- ns-3 spectrum: <https://www.nsnam.org/docs/models/html/spectrum.html>
- Módulo académico IEEE 802.11ah de imec: <https://github.com/imec-idlab/IEEE-802.11ah-ns-3>
- Revisión técnica de Wi-Fi HaLow: <https://www.famaey.eu/papers/jnl-tian2021a.pdf>
- Material IEEE TG4g: <https://mentor.ieee.org/802.15/dcn/09/15-09-0738-00-004g-tg4g-proposals-common-parts-and-differences.pdf>
- TI CC1312R: <https://www.ti.com/product/CC1312R>
- Mediciones LoRa 915 MHz en mina de oro: <https://www.mdpi.com/1424-8220/22/22/8653>
- Mediciones multifrecuencia NIOSH: <https://stacks.cdc.gov/view/cdc/228212>
- Mediciones NIOSH alrededor de curvas: <https://stacks.cdc.gov/view/cdc/46801>
- Regulación MTC 915–928 MHz: <https://cdn.www.gob.pe/uploads/document/file/1861732/Resoluci%C3%B3n%20Ministerial%20nro%20373-2021-MTC/01.pdf>

---

## 22. Instrucción breve para el próximo asistente o investigador

Antes de editar:

1. leer este documento completo;
2. ejecutar `git status`, `git branch -vv` y `git log -5`;
3. comprobar el estado del PR 1;
4. no asumir que los YAML actuales contienen las decisiones recientes;
5. actualizar primero documentación y configuración;
6. conservar un escenario común de 50 ms para una comparación justa;
7. tratar MR-FSK a 10 ms como escenario optimizado adicional;
8. no afirmar soporte nativo de 802.15.4g o 802.11ah en ns-3;
9. no inventar parámetros de túnel o interferencia;
10. mantener trazabilidad de commit, versión, semilla y configuración en cada resultado.

**Siguiente acción técnica recomendada:** preparar un único commit de especificación que cambie las tecnologías y el tráfico, sin implementar todavía la simulación. Ese commit debe incluir los nuevos YAML, los cálculos de carga/tiempo en aire y la matriz de 48 ejecuciones. Después se debe decidir el nivel de fidelidad PHY antes de escribir los instaladores C++.
