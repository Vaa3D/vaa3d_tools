class ArgumentNullError(Exception):
    """
    Thrown to indicate that a parameter is None when a value is expected
    """

    def __init__(self, parameter_name: str):
        super().__init__(f"Parameter {parameter_name} is None. Non null value expected.")
