import importlib.util
import pathlib
import sys
import types
import unittest


SCRIPT_PATH = pathlib.Path(__file__).with_name("texture_budget_batch_3.py")


def load_batch_module():
    sys.modules.setdefault("unreal", types.SimpleNamespace(Texture=object))
    spec = importlib.util.spec_from_file_location("texture_budget_batch_3", SCRIPT_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class TextureBudgetBatch3Test(unittest.TestCase):
    def test_batch_settings_match_planned_assets(self):
        module = load_batch_module()

        self.assertEqual(
            module.BATCH_SETTINGS,
            {
                "/Game/PvPArena/Textures/T_Metal_1_ORM": {"max_texture_size": 1024},
                "/Game/PvPArena/Textures/T_Trim_Metal_2_ORM": {"max_texture_size": 512},
                "/Game/PvPArena/Textures/T_Concrete_1_AO": {"max_texture_size": 512},
                "/Game/PvPArena/Textures/T_SurfaceImperfections_1_M": {"max_texture_size": 512},
                "/Game/PvPArena/Textures/Trim_Textures/T_Trim_Pipe_1_B": {"max_texture_size": 1024},
            },
        )


if __name__ == "__main__":
    unittest.main()
