<?php
namespace Core\Routing;

use Core\Support\Str;

class RouteAction {
    public static function parse($uri, $action)
    {
        if (is_null($action)) {
            return static::missingAction($uri);
        }

        if (is_callable($action)) {
            return ['uses' => $action];
        } elseif (! isset($action['uses'])) {
            $action['uses'] = static::findCallable($action);
        }

        if (is_string($action['uses']) && ! Str::contains($action['uses'], '@')) {
            $action['uses'] = static::makeInvokable($action['uses']);
        }
    }

    protected static function findCallable(array $action)
    {
        return Arr::first($action, function ($value, $key) {
            return is_callable($value) && is_numeric($key);
        });
    }

    protected static function missingAction($uri)
    {
        return ['uses' => function () use ($uri) {
            throw new \Exception("Route for [{$uri}] has no action.");
        }];
    }

    protected static function makeInvokable($action)
    {
        if (! method_exists($action, '__invoke')) {
            throw new \Exception("Invalid route action: [{$action}].");
        }

        return $action.'@__invoke';
    }
}