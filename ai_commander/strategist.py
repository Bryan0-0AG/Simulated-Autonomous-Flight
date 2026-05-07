from google import genai
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
            self.client = None
        else:
            # New SDK: Client Initialization
            self.client = genai.Client(api_key=self.api_key)
            self.model_id = 'gemini-2.0-flash'

    def generate_strategy(self, swarm_status):
        """
        Receives a dictionary with the swarm status and returns a strategy in JSON.
        """
        prompt = f"""
        You are the Commander of a Swarm of Autonomous Drones on an urban logistics mission.
        
        CURRENT SWARM STATUS:
        {json.dumps(swarm_status, indent=2)}
        
        TASK:
        Analyze the status and issue tactical orders. 
        You must respond EXCLUSIVELY in JSON format with the following structure:
        {{
            "analysis": "Brief explanation of the situation",
            "global_order": "General instruction for the swarm",
            "priority_zones": [[x1, y1], [x2, y2]],
            "safety_level": "low/medium/high"
        }}
        """

        if not self.client:
            # Mock mode in case there is no API Key yet
            return {
                "analysis": "Offline mode: Maintaining standard formation.",
                "global_order": "MAINTAIN_POSITION",
                "priority_zones": [],
                "safety_level": "medium"
            }

        try:
            # New SDK: models.generate_content call
            response = self.client.models.generate_content(
                model=self.model_id,
                contents=prompt
            )
            # Clean the response in case the LLM adds markdown (```json ...)
            text = response.text.replace('```json', '').replace('```', '').strip()
            return json.loads(text)
        except Exception as e:
            print(f"[ERROR] Failed to contact strategist: {e}")
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
