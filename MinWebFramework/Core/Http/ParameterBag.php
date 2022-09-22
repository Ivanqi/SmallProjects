<?php
namespace Core\Http;

class ParameterBag {
    protected $parameters = [];

    public function __construct(array $parameters = [])
    {
        $this->parameters = $parameters;
    }

    public function all(): array
    {
        return $this->parameters;
    }

    public function keys(): array
    {
        return array_keys($this->parameters);
    }

    public function replace(array $parameters = []): void
    {
        $this->parameters = $parameters;
    }

    public function add(array $parameters = []): void
    {
        $this->parameters = array_replace($this->parameters, $parameters);
    }

    public function get(string $key, $default = null)
    {
        return (array_key_exists($key, $this->parameters) ? $this->parameters[$key] : $default);
    }

    public function set(string $key, $value): void
    {
        $this->parameters[$key] = $value;
    }

    public function has($key): bool
    {
        return array_key_exists($key, $this->parameters);
    }

    public function remove($key): void
    {
        unset($this->parameters[$key]);
    }

    public function count(): int
    {
        return count($this->parameters);
    }
}