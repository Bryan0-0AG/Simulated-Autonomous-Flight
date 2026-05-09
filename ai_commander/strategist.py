from google import genai
import json
import os

# Keywords that indicate a model is NOT useful for text strategy generation
EXCLUDED_KEYWORDS = ["tts", "image", "robotics", "deep-research", "computer-use", "lyria", "banana", "gemma"]


def _model_priority(name):
    """
    Returns a sorting key so lite models come first (cheapest),
    then flash, then pro (most expensive / last resort).
    """
    if "lite" in name:
        return 0
    elif "flash" in name:
        return 1
    elif "pro" in name:
        return 2
    return 3


def discover_models(client):
    """
    Queries the Gemini API for all models that support generateContent,
    filters out non-text models, and returns them sorted cheapest-first.
    """
    try:
        all_models = client.models.list()
        valid = []
        for m in all_models:
            # Only keep models that can generate text content
            if "generateContent" not in m.supported_actions:
                continue
            # Skip specialized models (TTS, image gen, robotics, etc.)
            name_lower = m.name.lower()
            if any(kw in name_lower for kw in EXCLUDED_KEYWORDS):
                continue
            # Strip "models/" prefix for the API call
            model_id = m.name.replace("models/", "")
            valid.append(model_id)

        # Sort: lite first, then flash, then pro
        valid.sort(key=_model_priority)
        return valid
    except Exception as e:
        print(f"[STRATEGIST] Error al descubrir modelos: {e}")
        # Hardcoded fallback in case the list endpoint itself fails
        return ["gemini-2.0-flash-lite", "gemini-2.0-flash", "gemini-2.5-flash"]


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
            self.model_cascade = []
        else:
            # New SDK: Client Initialization
            self.client = genai.Client(api_key=self.api_key)
            # Dynamically discover and sort all available text models
            self.model_cascade = discover_models(self.client)
            print(f"[STRATEGIST] {len(self.model_cascade)} modelos descubiertos: {self.model_cascade}")

        # Track which model responded last for logging
        self.last_model_used = None

    def generate_strategy(self, swarm_status):
        """
        Receives a dictionary with the swarm status and returns a strategy in JSON.
        Tries each model in MODEL_CASCADE until one responds successfully.
        """
        prompt = f"""
        You are the Commander of a Swarm of Autonomous Drones on an urban logistics mission.
        
        CURRENT SWARM STATUS:
        {json.dumps(swarm_status)}
        
        TASK:
        Analyze the status and issue tactical orders. 
        You must respond EXCLUSIVELY in JSON format with the following structure:
        {{
            "commands": [
                {{"id": 0, "action": "MOVE", "target": [600, 700]}},
                {{"id": 1, "action": "RESHAPE", "cols": 4}}
            ]
        }}
        """

        if not self.client:
            # Mock mode in case there is no API Key yet
            return {"commands": []}

        # Cascade through models until one works
        for model_id in self.model_cascade:
            try:
                response = self.client.models.generate_content(
                    model=model_id,
                    contents=prompt
                )
                # Clean the response in case the LLM adds markdown
                text = response.text.replace('```json', '').replace('```', '').strip()
                result = json.loads(text)

                # Success: log which model answered and return
                self.last_model_used = model_id
                print(f"[STRATEGIST] Modelo exitoso: {model_id}")
                return result

            except Exception as e:
                error_str = str(e)
                # If quota exhausted (429), try the next model silently
                if "429" in error_str or "RESOURCE_EXHAUSTED" in error_str:
                    print(f"[STRATEGIST] {model_id} agotado. Probando siguiente...")
                    continue
                # For any other error, log it and also try next
                print(f"[STRATEGIST] {model_id} fallo: {error_str[:100]}")
                continue

        # All models exhausted: return empty fallback
        print("[STRATEGIST] TODOS los modelos agotados. Usando fallback local.")
        self.last_model_used = "FALLBACK_LOCAL"
        return {"commands": []}

# Quick test (internal use only)
if __name__ == "__main__":
    test_status = {
        "matrices": [
            {"id": 0, "p": [100, 200], "t": [500, 500], "s": 1, "d": 20},
            {"id": 1, "p": [300, 400], "t": [800, 200], "s": 0, "d": 15}
        ]
    }
    strategist = DroneStrategist()
    result = strategist.generate_strategy(test_status)
    print(f"\nModelo usado: {strategist.last_model_used}")
    print(f"Resultado: {json.dumps(result, indent=2)}")
