from typing import Any, Callable


class Container:
    def __init__(self) -> None:
        self._providers: dict[str, tuple[Callable[[], Any], bool]] = {}
        self._singletons: dict[str, Any] = {}

    def register(
        self,
        name: str,
        factory: Callable[[], Any],
        singleton: bool = False,
    ) -> None:
        """
        Register a provider under the given name.

        Args:
            name: Unique identifier for this dependency.
            factory: Zero-argument callable that produces the dependency.
            singleton: If True, the factory is called once
        """
        self._providers[name] = (factory, singleton)
        if name in self._singletons:
            del self._singletons[name]

    def resolve(self, name: str) -> Any:
        """
        Resolve a dependency by name.

        For singletons, the factory is invoked on first call and the
        result is cached. For factories, a new instance is created
        every time.

        Args:
            name: The identifier used during registration.

        Returns:
            The resolved dependency instance.

        Raises:
            KeyError: If no provider is registered under the given name.
        """
        if name not in self._providers:
            raise KeyError(f"Container: no provider registered for '{name}'")

        factory, is_singleton = self._providers[name]

        if is_singleton:
            if name not in self._singletons:
                self._singletons[name] = factory()
            return self._singletons[name]

        return factory()

    def has(self, name: str) -> bool:
        return name in self._providers
