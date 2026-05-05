import google.generativeai as genai
import json
import os

class DroneStrategist:
    def __init__(self, api_key=None):
        env_path = os.path.join(os.path.dirname(__file__), "..", ".env")
        if os.path.exists(env_path):
            with open(env_path, "r") as f:
                for line in f:
                    if "GEMINI_API_KEY=" in line:
                        os.environ["GEMINI_API_KEY"] = line.strip().split("=", 1)[1]

        self.api_key = api_key or os.getenv("GEMINI_API_KEY")
        if not self.api_key:
            print("[ADVERTENCIA] No se encontro GEMINI_API_KEY. El estratega operara en modo MOCK.")
            self.model = None
        else:
            genai.configure(api_key=self.api_key)
            self.model = genai.GenerativeModel('gemini-3-flash-preview')

    def generate_strategy(self, swarm_status):
        """
        Recibe un diccionario con el estado del enjambre y devuelve una estrategia en JSON.
        """
        prompt = f"""
        Eres el Comandante de un Enjambre de Drones Autónomos en una misión de logística urbana.
        
        ESTADO ACTUAL DEL ENJAMBRE:
        {json.dumps(swarm_status, indent=2)}
        
        TAREA:
        Analiza el estado y emite órdenes tácticas. 
        Debes responder EXCLUSIVAMENTE en formato JSON con la siguiente estructura:
        {{
            "analysis": "Breve explicación de la situación",
            "global_order": "Instrucción general para el enjambre",
            "priority_zones": [[x1, y1], [x2, y2]],
            "safety_level": "low/medium/high"
        }}
        """

        if not self.model:
            # Modo Mock por si no hay API Key todavía
            return {
                "analysis": "Modo offline: Manteniendo formación estándar.",
                "global_order": "MANTENER_POSICION",
                "priority_zones": [],
                "safety_level": "medium"
            }

        try:
            response = self.model.generate_content(prompt)
            # Limpiamos la respuesta por si el LLM añade markdown (```json ...)
            text = response.text.replace('```json', '').replace('```', '').strip()
            return json.loads(text)
        except Exception as e:
            print(f"[ERROR] Al consultar al LLM: {e}")
            return {"error": "Failed to contact strategist"}

# Ejemplo de uso rápido (solo para pruebas internas)
if __name__ == "__main__":
    test_status = {
        "total_drones": 5000,
        "avg_battery": 45.5,
        "active_missions": 12,
        "weather": "windy"
    }
    strategist = DroneStrategist()
    print(strategist.generate_strategy(test_status))
