<?php
namespace Core\Container;

class Container {
    // 绑定回调函数
    public $binds = [];

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
		return function ($container) use ($abstract, $concrete) {
			$method = ($abstract == $concrete) ? 'build' : 'make';
			return $container->$method($concrete);
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
		$concrete = $this->getConcrete($abstract);
		if ($this->isBuildable($abstract, $concrete)) {
			$obj = $this->build($concrete);
		} else {
			$obj = $this->make($concrete);
		}
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
		return (array)$deps;
	}

    // 获取参数的类型类名字
	public function resolveClass(\ReflectionParameter $refParam): string 
    {
		return $this->make($refParam->getClass()->name);
	}
}