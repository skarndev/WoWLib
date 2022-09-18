from typing import List, Tuple, Union, Set


class CxxType:
    """
    Type descriptor for C++ types used to define structures.
    """
    __slots__ = \
        (
            'name',
            'namespace',
            'headers',
            'is_template',
            'template_args',
            'n_template_args'
        )

    name: str
    """ The name of type in C++. """

    namespace: str
    """ Namespace the type is found in. """

    headers: Set[str]
    """ Headers necessary to include to obtain a complete type. """

    is_template: bool
    """ True if a represented type is a C++ template. """

    template_args: List[str]
    """ Strings used as template arguments to specialize a template type. """

    n_template_args: int
    """ Number of template arguments. """
    def __init__(self
                 , name: str
                 , namespace: str = 'std'
                 , headers: Set[str] = {'cstdint'}
                 , template_args: List[str] = []):
        """
        Initialize BuiltinType class.
        :param name: The name of type  in C++ without namespace.
        :param namespace: Namespace the type is found in.
        :param headers: Headers necessary to include to obtain a complete type.
        It's user's responsibility to ensure type completeness.
        :param template_args: Strings or CxxTypes used as template arguments to specialize a template type.
        """
        self.name = name
        self.namespace = namespace
        self.headers = headers
        self.is_template = bool(template_args)
        self.template_args = template_args
        self.n_template_args = len(template_args)

    def __getitem__(self, dim: int) -> 'CxxType':
        """
        Used for creating array permutations of type.
        :param dim: dimension of array of type.
        :return: Mutated CxxType with pre-baked array dimensions.
        """
        new_headers = self.headers.copy()
        new_headers.add('array')

        return CxxType(f"array", headers=new_headers, template_args=[self.full_typename, str(dim)])

    def __lshift__(self, args: Union[str, 'CxxType'] | Tuple[Union[str, 'CxxType']]) -> 'CxxType':
        """
        Used for creating template specializations of type.
        :param args: Single or multiple template arguments as string or other CxxTypes.
        :return: Mutated CxxType with specialized template parameters.
        :raises: RuntimeError if incorrect number of templates argument is given.
        """

        # multiple arguments
        if isinstance(args, tuple):
            if self.n_template_args != len(args):

                if self.namespace:
                    typename = f"{self.namespace}::{self.name}"
                else:
                    typename = self.name

                raise RuntimeError(f"Wrong number of template arguments for type \"{typename}\","
                                   f" expected {self.n_template_args}, got {len(args)}.")

            arg_headers = set()
            for arg in args:
                if isinstance(arg, CxxType):
                    arg_headers |= arg.headers

            return CxxType(self.name, namespace=self.namespace, headers=self.headers | arg_headers
                           , template_args=[arg.full_typename if isinstance(arg, CxxType) else arg for arg in args])
        # singular argument
        else:
            args_headers = set()
            if isinstance(args, CxxType):
                args_headers |= args.headers

            return CxxType(self.name, namespace=self.namespace, headers=self.headers | args_headers
                           , template_args=[args.full_typename if isinstance(args, CxxType) else args])

    @property
    def full_typename(self) -> str:
        """
        Generate C++ compatible fully-qualified typename.
        :return: String containing the resulted C++ typename.
        :raises: RuntimeError if incorrect number of template arguments is given.
        """

        if self.namespace:
            typename = f"{self.namespace}::{self.name}"
        else:
            typename = self.name

        if self.is_template:
            if not len(self.template_args) == self.n_template_args:
                raise RuntimeError(f"Wrong number of template arguments for type \"{typename}\","
                                   f" expected {self.n_template_args}, got {len(self.template_args)}")

            typename += "<{}>".format(", ".join(self.template_args))

        return typename


class CxxNumericType(CxxType):
    """ Type descriptor for C++ numeric types. """

    __slots__ = \
        (
            'min',
            'max',
            'is_integral'
        )

    min: int | float
    """ Minimum value of type. """

    max: int | float
    """ Maximum value of type. """

    is_integral: bool
    """ True if type is integral. """

    def __init__(self
                 , name: str
                 , min_val: int | float
                 , max_val: int | float
                 , namespace: str = 'std'
                 , headers: Set[str] = {'cstdint'}
                 , template_args: List[str] = []
                ):
        """
        Initialize CxxNumericType class.
        :param name: The name of type  in C++ without namespace.
        :param min_val: Minimum value of type.
        :param max_val: Maximum value of type.
        :param namespace: Namespace the type is found in.
        :param headers: Headers necessary to include to obtain a complete type.
        It's user's responsibility to ensure type completeness.
        :param template_args: Strings or CxxTypes used as template arguments to specialize a template type.
        """
        super().__init__(name, namespace, headers, template_args)
        self.min = min_val
        self.max = max_val

        assert(type(min_val) == type(max_val))

        self.is_integral = isinstance(min_val, int)

    def check_value_bounds(self, val: int | float) -> bool:
        """
        Check if value is within the allowed bounds.
        :param val: Value to test.
        :return: True if val is within the type bounds, else False.
        """
        if self.is_integral:
            if not isinstance(val, int):
                return False

            return val in range(self.min, self.max + 1)

        else:
            if not isinstance(val, float):
                return False

            return self.min < val <= self.max


