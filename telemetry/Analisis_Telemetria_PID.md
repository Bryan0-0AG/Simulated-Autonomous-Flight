# Reporte de Análisis de Telemetría: Controlador PID y Gravedad

**Fecha:** 17 de Abril de 2026  
**Log Analizado:** `telemetry_2026-04-17_12-54-27.csv`  

## 1. Resumen Ejecutivo

La reciente inclusión de los términos individuales del PID (`P`, `I`, `D`) en los registros de telemetría permitió realizar una radiografía exacta del comportamiento del dron en vuelo. 

El análisis de los datos revela **por qué el dron nunca alcanza su objetivo vertical (`target_y`)**, quedándose suspendido sistemáticamente a ~48.5 píxeles por debajo de su meta. Este fenómeno se conoce en la teoría de control como **"Error en estado estacionario"** (Steady-State Error) y es una consecuencia directa de la interacción entre el controlador PID y la fuerza constante de la gravedad.

---

## 2. Evidencia en los Datos (El "Steady-State Error")

Tomando como referencia el **Dron 0** al final de la simulación (tiempo > 40s), observamos que el sistema entra en un estado de equilibrio donde ya no se mueve, pero no está en el target:

| Variable | Valor en Estado Estacionario (t=47s) |
| :--- | :--- |
| **Posición Y (`pos_y`)** | `98.46` |
| **Objetivo Y (`target_y`)**| `147.0` |
| **Error Y (`error_y`)** | **`48.54`** |

### Desglose del Controlador PID en Y (Thrust)
Al observar qué está haciendo el PID en ese preciso instante para intentar subir esos 48.54 píxeles que le faltan, los registros muestran:

*   **Término Proporcional (`thrust_pid_p`)**: `9.708`
*   **Término Integral (`thrust_pid_i`)**: `0.1` *(Saturado al máximo)*
*   **Término Derivativo (`thrust_pid_d`)**: `0.0` *(Sin movimiento = sin derivada)*
*   **Suma Total de Empuje (Thrust)**: **`9.808`**

---

## 3. Diagnóstico Matemático: La Gravedad Oculta

¿Por qué el empuje total se estabiliza exactamente en **`9.808`**? 
Porque ese es el valor exacto de la aceleración de la **gravedad (`9.81`)** en la simulación. El dron ha encontrado el punto exacto donde su empuje cancela la gravedad, permitiéndole "flotar".

Sin embargo, el sistema obliga al término **Proporcional** a hacer todo el trabajo pesado. Veamos la matemática detrás de esto:

1. **El límite del Integral:** El límite anti-windup (`max_integral = 20`) multiplicado por la constante integral (`KI = 0.005`) da como resultado un empuje máximo del término integral de `0.1`. Es decir, el término integral no tiene "fuerza" suficiente para cargar con el peso del dron.
2. **El esfuerzo del Proporcional:** Como el dron necesita `9.81` para no caer, y el integral solo aporta `0.1`, el término proporcional *debe* aportar los `9.71` restantes de forma indefinida.
3. **El error resultante:** Dado que la constante proporcional es `KP = 0.2`, la única forma de que el término proporcional genere `9.71` de empuje es existiendo un error permanente de posición:
   
   $$Error \times KP = Empuje$$
   $$Error \times 0.2 = 9.71$$
   $$Error = 48.55 \text{ píxeles}$$

Esta es la razón matemática exacta por la cual todos los drones se quedan a 48.5 píxeles por debajo de su destino.

---

## 4. Otros hallazgos en la Telemetría

Se detectó un bug de logging en las columnas `thrust` y `angle` del archivo CSV. Los valores impresos rondan el `5.04e-43` (basura de memoria).
*   **Causa:** Se agregaron las variables `thrust` y `angle` en el `struct Body`, pero el controlador (`Controller::update`) deposita los cálculos reales dentro de la sub-estructura `body.actuator_output`. El registro en `main.cpp` está leyendo las variables sin inicializar en lugar de los resultados del actuador.

---

## 5. Soluciones Propuestas

### Solución A: Feedforward de Gravedad (Recomendada)
En la industria de los drones, la gravedad no se vence con el PID, sino que se asume como una constante base. Se debe sumar la gravedad directamente a la salida del actuador en el controlador:

```cpp
// En controller.cpp
body.actuator_output.thrust = body.thrust_pid.p + body.thrust_pid.i + body.thrust_pid.d + 9.81f; // Feedforward
```
*Beneficio:* El PID solo se preocupará por acelerar o frenar, permitiendo que el error llegue a `0` sin esfuerzo y manteniendo el dron estable en el punto exacto.

### Solución B: Corregir el Registro (Logging)
En `main.cpp`, modificar los argumentos pasados al `logger`:

```cpp
// Cambiar:
body.thrust,
body.angle,

// Por:
body.actuator_output.thrust,
body.actuator_output.angle,
```

## Conclusión
La instrumentación del código (logging avanzado) fue un éxito total. Demostró que el algoritmo PID funciona perfectamente según su configuración, y diagnosticó una carencia física (falta de compensación gravitatoria) que es estándar en la simulación de vuelo.
