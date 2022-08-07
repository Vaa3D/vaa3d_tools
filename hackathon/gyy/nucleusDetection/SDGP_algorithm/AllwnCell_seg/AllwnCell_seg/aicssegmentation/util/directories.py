import aicssegmentation

from pathlib import Path


class Directories:
    """
    Provides safe paths to common module directories
    """

    _module_base_dir = Path(aicssegmentation.__file__).parent

    @classmethod
    def get_assets_dir(cls) -> Path:
        """
        Path to the assets directory
        """
        return cls._module_base_dir / "assets"

    @classmethod
    def get_structure_config_dir(cls) -> Path:
        """
        Path to the structure json config directory
        """
        return cls._module_base_dir / "structure_wrapper_config"
