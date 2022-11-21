<?php
namespace Core\Routing;

use Core\Routing\Route;
use Core\Support\Arr;

class RouteCollection {
    protected $routes = [];
    protected $allRoutes = [];
    protected $nameList = [];
    protected $actionList = [];

    public function add($route)
    {
        $this->addToCollections($route);

        $this->addLookups($route);

        return $route;
    }

    public function get($method = null)
    {
        return is_null($method) ? $this->getRoutes() : Arr::get($this->routes, $method, []);
    }

    public function getRoutes()
    {
        return array_values($this->allRoutes);
    }

    protected function addToCollections($route): void
    {
        $domainAndUri = $route->domain().$route->uri();

        foreach ($route->methods() as $method) {
            $this->routes[$method][$domainAndUri] = $route;
        }

        $this->allRoutes[$method.$domainAndUri] = $route;
    }

    protected function addLookups($route): void
    {
        $action = $route->getAction();

        if (isset($action['as'])) {
            $this->nameList[$action['as']] = $route;
        }

        if (isset($action['controller'])) {
            $this->addToActionList($action, $route);
        }
    }
    

    protected function addToActionList(array $action, $route): void
    {
        $this->actionList[trim($action['controller'], '\\')] = $route;
    }

    protected function matchAgainstRoutes(array $routes, $request, $includingMethod = true)
    {
        return Arr::first($routes, function ($value) use ($request, $includingMethod) {
            return $value->matches($request, $includingMethod);
        });
    }

    protected function checkForAlternateVerbs($request)
    {
        $methods = array_diff(Router::$verbs, [$request->getMethod()]);

        // Here we will spin through all verbs except for the current request verb and
        // check to see if any routes respond to them. If they do, we will return a
        // proper error response with the correct headers on the response string.
        $others = [];

        foreach ($methods as $method) {
            if (! is_null($this->matchAgainstRoutes($this->get($method), $request, false))) {
                $others[] = $method;
            }
        }

        return $others;
    }

    protected function getRouteForMethods($request, array $methods)
    {
        if ($request->method() == 'OPTIONS') {
            return (new Route('OPTIONS', $request->path(), function () use ($methods) {
                return new Response('', 200, ['Allow' => implode(',', $methods)]);
            }))->bind($request);
        }

        $this->methodNotAllowed($methods);
    }

    protected function methodNotAllowed(array $others)
    {
        throw new \Exception($others);
    }

    public function match($request)
    {
        $routes = $this->get($request->getMethod());

        // First, we will see if we can find a matching route for this current request
        // method. If we can, great, we can just return it so that it can be called
        // by the consumer. Otherwise we will check for routes with another verb.
        $route = $this->matchAgainstRoutes($routes, $request);

        if (!is_null($route)) {
            return $route->bind($request);
        }

        throw new \Exception('Not Match Request');
    }
}