<?php
namespace Core\Routing;

class ControllerDispatcher {
    protected $container;

    public function __construct($container)
    {
        $this->container = $container;
    }

    public function dispatch(Route $route, $controller, $method)
    {
        $parameters = [];
        
        return $controller->{$method}(...array_values($parameters));
    }
}