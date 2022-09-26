<?php
namespace Core\Routing;

use Core\Container\Container;
use Core\Routing\Route;
use Core\Routing\RouteGroup;
use Core\Http\Response;

class Router {
    protected $events;
    protected $container;
    protected $routes;
    protected $current;
    protected $currentRequest;
    protected $middleware = [];
    protected $groupStack = [];
    protected $patterns = [];


    public static $verbs = ['GET', 'HEAD', 'POST', 'PUT', 'PATCH', 'DELETE', 'OPTIONS'];

    public function __construct(Container $container = null)
    {
        $this->routes = new RouteCollection;
        $this->container = $container ?: new Container;
    }

    public function get($uri, $action = null)
    {
        return $this->addRoute(['GET', 'HEAD'], $uri, $action);
    }

    public function post($uri, $action = null)
    {
        return $this->addRoute('POST', $uri, $action);
    }

    protected function addRoute($methods, $uri, $action)
    {
        return $this->routes->add($this->createRoute($methods, $uri, $action));
    }

    protected function actionReferencesController($action)
    {
        if (! $action instanceof \Closure) {
            return is_string($action) || (isset($action['uses']) && is_string($action['uses']));
        }

        return false;
    }

    protected function createRoute($methods, $uri, $action)
    {
        // If the route is routing to a controller we will parse the route action into
        // an acceptable array format before registering it and creating this route
        // instance itself. We need to build the Closure that will call this out.
        if ($this->actionReferencesController($action)) {
            // controller@action类型的路由在这里要进行转换
            $action = $this->convertToControllerAction($action);
        }

        $route = $this->newRoute(
            $methods, $this->prefix($uri), $action
        );

        // If we have groups that need to be merged, we will merge them now after this
        // route has already been created and is ready to go. After we're done with
        // the merge we will be ready to return the route back out to the caller.
        if ($this->hasGroupStack()) {
            $this->mergeGroupAttributesIntoRoute($route);
        }

        $this->addWhereClausesToRoute($route);

        return $route;
    }

    public function mergeWithLastGroup(array $new): array
    {
        return RouteGroup::merge($new, end($this->groupStack));
    }

    /**
     * 将group stack 与控制器操作合并
     */
    protected function mergeGroupAttributesIntoRoute(Route $route)
    {
        $route->setAction($this->mergeWithLastGroup($route->getAction()));
    }

    /**
     * 
     */
    public function hasGroupStack()
    {
        return ! empty($this->groupStack);
    }

    /**
     * 将基于控制器的路由动作添加到action数组
     */
    protected function convertToControllerAction($action): array
    {
        if (is_string($action)) {
            $action = ['uses' => $action];
        }

        // Here we'll merge any group "uses" statement if necessary so that the action
        // has the proper clause for this property. Then we can simply set the name
        // of the controller on the action and return the action array for usage.
        if (! empty($this->groupStack)) {
            $action['uses'] = $this->prependGroupNamespace($action['uses']);
        }

        // Here we will set this controller name on the action array just so we always
        // have a copy of it for reference if we need it. This can be used while we
        // search for a controller name or do some other type of fetch operation.
        $action['controller'] = $action['uses'];

        return $action;
    }

    /**
     * 将最后一个组名称空间放在use子句前面
     */
    protected function prependGroupNamespace(string $class): string
    {
        $group = end($this->groupStack);

        return isset($group['namespace']) && strpos($class, '\\') !== 0
                ? $group['namespace'].'\\'.$class : $class;
    }

    protected function newRoute($methods, $uri, $action)
    {
        return (new Route($methods, $uri, $action))
                    ->setRouter($this)
                    ->setContainer($this->container);
    }

    protected function addWhereClausesToRoute($route)
    {
        $route->where(array_merge(
            $this->patterns, isset($route->getAction()['where']) ? $route->getAction()['where'] : []
        ));

        return $route;
    }

    protected function prefix($uri)
    {
        // return trim(trim($this->getLastGroupPrefix(), '/').'/'.trim($uri, '/'), '/') ?: '/';
        return $uri;
    }

    public function group(array $attributes, mixed $routes): void
    {
        $this->updateGroupStack($attributes);

        // Once we have updated the group stack, we'll load the provided routes and
        // merge in the group's attributes when the routes are created. After we
        // have created the routes, we will pop the attributes off the stack.
        $this->loadRoutes($routes);

        array_pop($this->groupStack);
    }

    protected function updateGroupStack(array $attributes): void
    {
        if (! empty($this->groupStack)) {
            $attributes = RouteGroup::merge($attributes, end($this->groupStack));
        }

        $this->groupStack[] = $attributes;
    }

    protected function loadRoutes(mixed $routes): void
    {
        if ($routes instanceof \Closure) {
            $routes($this);
        } else {
            $router = $this;

            require $routes;
        }
    }

    public function dispatch($request)
    {
        $this->currentRequest = $request;

        return $this->dispatchToRoute($request);
    }

    protected function findRoute($request)
    {
        $this->current = $route = $this->routes->match($request);
        return $route;
    }

    public function dispatchToRoute($request)
    {
        // First we will find a route that matches this request. We will also set the
        // route resolver on the request so middlewares assigned to the route will
        // receive access to this route instance for checking of the parameters.
        $route = $this->findRoute($request);

        // $request->setRouteResolver(function () use ($route) {
        //     return $route;
        // });

        // $this->events->dispatch(new Events\RouteMatched($route, $request));

        $response = $this->runRouteWithinStack($route, $request);

        return $this->prepareResponse($request, $response);
    }

    protected function runRouteWithinStack($route, $request)
    {
        // $shouldSkipMiddleware = $this->container->bound('middleware.disable') &&
        //                         $this->container->make('middleware.disable') === true;

        // $middleware = $shouldSkipMiddleware ? [] : $this->gatherRouteMiddleware($route);

        
        return $this->prepareResponse($request, $route->run());
    }

    public function prepareResponse($request, $response)
    {
        if (!$response instanceof Response) {
            $response = new Response($response);
        }

        return $response->prepare($request);
    }
}