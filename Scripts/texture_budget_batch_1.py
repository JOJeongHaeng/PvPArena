import unreal


BATCH_SETTINGS = {
    "/Game/PvPArena/Textures/Trim_Textures/T_Panel_3_ORM": {"max_texture_size": 512},
    "/Game/PvPArena/Textures/Trim_Textures/T_Trim_Pipe_1_ORM": {"max_texture_size": 512},
    "/Game/PvPArena/Textures/Trim_Textures/T_Props_B_ORM": {"max_texture_size": 512},
    "/Game/PvPArena/Textures/Trim_Textures/T_Props_A_ORM": {"max_texture_size": 512},
    "/Game/PvPArena/Textures/Trim_Textures/T_Wall_Trim_2_N": {"max_texture_size": 1024},
    "/Game/PvPArena/Textures/Trim_Textures/T_Panel_2_N": {"max_texture_size": 1024},
    "/Game/PvPArena/Textures/Trim_Textures/T_Panel_Trim7_N": {"max_texture_size": 1024},
}


def set_if_different(asset: unreal.Texture, property_name: str, value: int) -> bool:
    current_value = asset.get_editor_property(property_name)
    if current_value == value:
        unreal.log(f"{asset.get_path_name()}: {property_name} already {value}")
        return False

    asset.set_editor_property(property_name, value)
    unreal.log(f"{asset.get_path_name()}: {property_name} {current_value} -> {value}")
    return True


def main() -> None:
    changed_assets = []

    for asset_path, settings in BATCH_SETTINGS.items():
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not asset:
            raise RuntimeError(f"Failed to load asset: {asset_path}")

        dirty = False
        for property_name, value in settings.items():
            dirty = set_if_different(asset, property_name, value) or dirty

        if dirty:
            unreal.EditorAssetLibrary.save_loaded_asset(asset)
            changed_assets.append(asset_path)

    unreal.log(f"texture_budget_batch_1 complete; changed {len(changed_assets)} assets")
    for asset_path in changed_assets:
        unreal.log(f"changed: {asset_path}")


if __name__ == "__main__":
    main()
