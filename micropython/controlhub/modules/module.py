"""module.py -- Base class + global registry for on-board modules.

Mirrors the intrusive Module linked list in src/modules/Module.{h,cpp}: every
module registers itself on construction so ControlHub can route an incoming
frame to the module whose (module_type, id) matches.

The shared PCA9685 instance (used by every Motor) is stashed here by
ControlHub.begin(); modules fetch it lazily in their own begin().
"""

_REGISTRY = []          # all constructed modules, in construction order
_SHARED = {"pca": None, "hub": None}


def register(module):
    _REGISTRY.append(module)


def registry():
    return _REGISTRY


def set_pca(pca):
    _SHARED["pca"] = pca


def get_pca():
    return _SHARED["pca"]


def set_hub(hub):
    _SHARED["hub"] = hub


def get_hub():
    return _SHARED["hub"]


class Module:
    """Base class: subclasses set module_type / id and implement handle_frame."""

    def __init__(self):
        register(self)

    # Subclasses override these.
    def module_type(self):
        return 0

    def id(self):
        return 0

    def begin(self):
        pass

    def handle_frame(self, frame, hub):
        pass
