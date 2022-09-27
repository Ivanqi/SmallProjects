<?php
namespace Core\Container;

class Container {
    // 绑定回调函数
    public $binds = [];
	protected $with = [];

    protected static $instance;

    public static function setInstance($container = null)
    {
        return static::$instance = $container;
    }

    public static function getInstance()
    {
        if (is_null(static::$instance)) {
            static::$instance = new static;
        }

        return static::$instance;
    }

	public function singleton($abstract, $concrete = null)
    {
        $this->bind($abstract, $concrete, true);
    }

    // 绑定接口 $abstract 与回调函数
    public function bind($abstract, $concrete = NULL, bool $shared = false): void
    {
        if (!$concrete instanceof \Closure) {
            $concrete = $this->getClosure($abstract, $concrete);
        }

        $this->binds[$abstract] = compact('concrete', 'shared');
    }

    // 获取回调函数
	public function getClosure($abstract, $concrete) {
		return function ($container, $parameters = []) use ($abstract, $concrete) {
            if ($abstract == $concrete) {
                return $container->build($concrete);
            }

            return $container->makeWith($concrete, $parameters);
        };
	}

	protected function getConcrete($abstract) {
		if (!isset($this->binds[$abstract])) {
			return $abstract;
		}
		return $this->binds[$abstract]['concrete'];
	}

    // 生成实例对象
	public function make($abstract)
    {
		return $this->resolve($abstract);
	}

	public function makeWith($abstract, array $parameters)
    {
        return $this->resolve($abstract, $parameters);
    }

	protected function resolve($abstract, $parameters = [])
    {
		$this->with[] = $parameters;

		$concrete = $this->getConcrete($abstract);
		if ($this->isBuildable($abstract, $concrete)) {
			$obj = $this->build($concrete);
		} else {
			$obj = $this->make($concrete);
		}

		array_pop($this->with);
		return $obj;
	}

    // 判断是否要用反射来实例化
	protected function isBuildable($abstract, $concrete): bool
    {
		return $concrete == $abstract || $concrete instanceof \Closure;
	}

    // 通过反射来实例化 $concrete 的对象
	public function build($concrete)
    {
		print_r($concrete);
		echo PHP_EOL;
		if ($concrete instanceof \Closure) {
			return $concrete($this);
		}

		$reflector = new \ReflectionClass($concrete);
		if (!$reflector->isInstantiable()) {
			$message = "Target [$concrete] is not instantiable.";
			throw new \Exception($message);
		}

		$constructor = $reflector->getConstructor();
		// 使用默认的构造函数
		if (is_null($constructor)) {
			return new $concrete;
		}

		$refParams = $constructor->getParameters();
		$instances = $this->getDependencies($refParams);
		return $reflector->newInstanceArgs($instances);
	}

    // 获取实例化对象时所需的参数
	public function getDependencies(array $refParams): array
    {
		$deps = [];
		foreach ($refParams as $refParam) {
			$dep = $refParam->getClass();
			if (is_null($dep)) {
				$deps[] = null;
			} else {
				$deps[] = $this->resolveClass($refParam);
			}
		}
		print_r($deps);
		return (array)$deps;
	}

    // 获取参数的类型类名字
	public function resolveClass(\ReflectionParameter $refParam): string 
    {
		print_r(['resolveClass', $refParam->getClass()->name]);
		return $this->make($refParam->getClass()->name);
	}
}