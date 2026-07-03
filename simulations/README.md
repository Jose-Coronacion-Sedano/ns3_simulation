# Simulacion basica en ns-3

Este repositorio contiene un ejemplo simple para iniciar con ns-3. La simulacion crea dos nodos conectados por un enlace punto a punto:

- Nodo 0: cliente UDP Echo.
- Nodo 1: servidor UDP Echo.
- El cliente envia paquetes al servidor y el servidor responde.

## Archivo principal

El codigo esta en:

```text
scratch/simulacion-basica.cc
```

## Como ejecutarlo

1. Copia el archivo `scratch/simulacion-basica.cc` dentro de la carpeta `scratch/` de tu instalacion de ns-3.
2. Desde la raiz de ns-3, ejecuta:

```bash
./ns3 run scratch/simulacion-basica
```

En Windows con PowerShell, si usas WSL o Git Bash, ejecuta el mismo comando dentro del entorno donde tengas instalado ns-3.

## Que se puede modificar facilmente

Dentro del codigo puedes cambiar:

- `DataRate`: velocidad del enlace.
- `Delay`: retardo del enlace.
- `MaxPackets`: cantidad de paquetes enviados.
- `Interval`: tiempo entre paquetes.
- `PacketSize`: tamano de cada paquete.
