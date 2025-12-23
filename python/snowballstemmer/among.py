from typing import Callable, Optional

class Among:
    def __init__(self, s: str, substring_i: int, result: int, method: Optional[Callable[..., bool]] = None) -> None:
        """
        @ivar s search string
        @ivar substring index to longest matching substring
        @ivar result of the lookup
        @ivar method method to use if substring matches
        """
        self.s = s
        self.substring_i = substring_i
        self.result = result
        self.method = method
