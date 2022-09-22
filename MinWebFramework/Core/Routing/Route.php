<?php
namespace Core\Routing;

use Core\Routing\Router;
use Core\Container\Container;

class Route {
    public $uri;
    public $methods;
    public $action;
    public $controller;
    public $compiled;
    public $wheres = [];

    protected $router;
    protected $container;
    
    public function __construct($methods, string $uri, $action)
    {
        $this->uri = $uri;
        $this->methods = (array) $methods;
        $this->action = $this->parseAction($action);

        if (in_array('GET', $this->methods) && ! in_array('HEAD', $this->methods)) {
            $this->methods[] = 'HEAD';
        }

        if (isset($this->action['prefix'])) {
            $this->prefix($this->action['prefix']);
        }
    }

    protected function parseAction($action)
    {
        return RouteAction::parse($this->uri, $action);
    }

    public function methods()
    {
        return $this->methods;
    }

    public function domain()
    {
        return isset($this->action['domain'])
                ? str_replace(['http://', 'https://'], '', $this->action['domain']) : null;
    }

    public function uri(): string
    {
        return $this->uri;
    }

    public function setUri(string $uri): Route
    {
        $this->uri = $uri;

        return $this;
    }

    public function getName(): string
    {
        return isset($this->action['as']) ? $this->action['as'] : null;
    }

    public function setRouter(Router $router)
    {
        $this->router = $router;

        return $this;
    }

    public function setContainer(Container $container)
    {
        $this->container = $container;

        return $this;
    }

    public function getAction()
    {
        return $this->action;
    }

    public function setAction(array $action)
    {
        $this->action = $action;

        return $this;
    }

    public function run()
    {
        $this->container = $this->container ?: new Container;

        try {
            if ($this->isControllerAction()) {
                return $this->runController();
            }

            return $this->runCallable();
        } catch (\Exception $e) {
            return $e->getResponse();
        }
    }

    public function parameters()
    {
        if (isset($this->parameters)) {
            return $this->parameters;
        }

        throw new \Exception('Route is not bound.');
    }

    public function parametersWithoutNulls()
    {
        return array_filter($this->parameters(), function ($p) {
            return ! is_null($p);
        });
    }

    protected function runCallable()
    {
        $callable = $this->action['uses'];

        return $callable(...array_values(
            $this->parametersWithoutNulls(), new \ReflectionFunction($this->action['uses'])
        ));
    }

    protected function isControllerAction()
    {
        return is_string($this->action['uses']);
    }

    protected function runController()
    {
        return (new ControllerDispatcher($this->container))->dispatch(
            $this, $this->getController(), $this->getControllerMethod()
        );
    }

    public function getController()
    {
        $class = $this->parseControllerCallback()[0];

        if (! $this->controller) {
            $this->controller = $this->container->make($class);
        }

        return $this->controller;
    }

    protected function getControllerMethod()
    {
        return $this->parseControllerCallback()[1];
    }

    protected function parseControllerCallback()
    {
        return Str::parseCallback($this->action['uses']);
    }

    public function where($name, $expression = null)
    {
        // foreach ($this->parseWhere($name, $expression) as $name => $expression) {
        //     $this->wheres[$name] = $expression;
        // }

        return $this;
    }

    protected function parseWhere($name, $expression)
    {
        return is_array($name) ? $name : [$name => $expression];
    }

    protected function compileRoute()
    {
        if (! $this->compiled) {
            $this->compiled = (new RouteCompiler($this))->compile();
        }

        return $this->compiled;
    }

    public function matches($request, $includingMethod = true)
    {
        print_r(['url', $this->uri]);
        $this->compileRoute();

        // foreach ($this->getValidators() as $validator) {
        //     if (! $includingMethod && $validator instanceof MethodValidator) {
        //         continue;
        //     }

        //     if (! $validator->matches($this, $request)) {
        //         return false;
        //     }
        // }

        return true;
    }

    public function bind($request)
    {
        $this->compileRoute();

        $this->parameters = (new RouteParameterBinder($this))->parameters($request);

        return $this;
    }
}