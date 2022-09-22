<?php
namespace Core\Routing;

class RouteCompiler {
    protected $route;

    public function __construct($route)
    {
        $this->route = $route;
    }

    protected function getOptionalParameters()
    {
        preg_match_all('/\{(\w+?)\?\}/', $this->route->uri(), $matches);
        return isset($matches[1]) ? array_fill_keys($matches[1], null) : [];
    }

    public function compile()
    {
        $optionals = $this->getOptionalParameters();

        $uri = preg_replace('/\{(\w+?)\?\}/', '{$1}', $this->route->uri());

        return (
            new BaseRoute($uri, $optionals, $this->route->wheres, [], $this->route->domain() ?: '')
        )->compile();
    }
}