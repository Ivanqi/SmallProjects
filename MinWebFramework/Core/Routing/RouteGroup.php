<?php
namespace Core\Routing;

use Core\Support\Arr;

class RouteGroup {
    public static function merge(array $new, array $old): array
    {
        if (isset($new['domain'])) {
            unset($old['domain']);
        }

        $new = array_merge(static::formatAs($new, $old), [
            'namespace' => static::formatNamespace($new, $old)
        ]);

        return array_merge_recursive(Arr::except(
            $old, ['namespace']
        ), $new);
    }

    protected static function formatNamespace(array $new, array $old): mixed
    {
        if (isset($new['namespace'])) {
            return isset($old['namespace'])
                    ? trim($old['namespace'], '\\').'\\'.trim($new['namespace'], '\\')
                    : trim($new['namespace'], '\\');
        }

        return isset($old['namespace']) ? $old['namespace'] : null;
    }
}