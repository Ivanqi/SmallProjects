<?php
namespace Core\Http;

class HeaderBag {
    protected $headers = [];
    protected $cacheControl = [];

    public function __construct(array $headers = [])
    {
        foreach ($headers as $key => $values) {
            $this->set($key, $values);
        }
    }

    public function __toString()
    {
        if (!$headers = $this->all()) {
            return '';
        }

        ksort($headers);
        $max = max(array_map('strlen', array_keys($headers))) + 1;
        $content = '';
        foreach ($headers as $name => $values) {
            $name = implode('-', array_map('ucfirst', explode('-', $name)));
            foreach ($values as $value) {
                $content .= sprintf("%-{$max}s %s\r\n", $name.':', $value);
            }
        }

        return $content;
    }

    public function all(): array
    {
        return $this->headers;
    }

    public function keys(): array
    {
        return array_keys($this->all());
    }

    public function replace(array $headers = []): void
    {
        $this->headers = array();
        $this->add($headers);
    }

    public function add(array $headers): void
    {
        foreach ($headers as $key => $values) {
            $this->set($key, $values);
        }
    }

    public function get(string $key, $default = null, bool $first = true)
    {
        $key = str_replace('_', '-', strtolower($key));
        $headers = $this->all();

        if (!array_key_exists($key, $headers)) {

            if (null === $default) {
                return $first ? null : array();
            }

            return $first ? $default : array($default);
        }


        if ($first) {
            return \count($headers[$key]) ? $headers[$key][0] : $default;
        }

        return $headers[$key];
    }

    public function set(string $key, $values, bool $replace = true): void
    {
        $key = str_replace('_', '-', strtolower($key));

        if (\is_array($values)) {
            $values = array_values($values);

            if (true === $replace || !isset($this->headers[$key])) {
                $this->headers[$key] = $values;
            } else {
                $this->headers[$key] = array_merge($this->headers[$key], $values);
            }
        } else {
            if (true === $replace || !isset($this->headers[$key])) {
                $this->headers[$key] = array($values);
            } else {
                $this->headers[$key][] = $values;
            }
        }

        if ('cache-control' === $key) {
            $this->cacheControl = $this->parseCacheControl(implode(', ', $this->headers[$key]));
        }
    }

    public function has(string $key): bool
    {
        return array_key_exists(str_replace('_', '-', strtolower($key)), $this->all());
    }

    public function contains(string $key, string $value): bool
    {
        return in_array($value, $this->get($key, null, false));
    }

    public function remove(string $key): bool
    {
        $key = str_replace('_', '-', strtolower($key));

        unset($this->headers[$key]);

        if ('cache-control' === $key) {
            $this->cacheControl = [];
        }
    }

    public function count(): int
    {
        return count($this->headers);
    }

    protected function parseCacheControl(string $header): array
    {
        $cacheControl = [];
        preg_match_all('#([a-zA-Z][a-zA-Z_-]*)\s*(?:=(?:"([^"]*)"|([^ \t",;]*)))?#', $header, $matches, PREG_SET_ORDER);
        foreach ($matches as $match) {
            $cacheControl[strtolower($match[1])] = isset($match[3]) ? $match[3] : (isset($match[2]) ? $match[2] : true);
        }

        return $cacheControl;
    }
}