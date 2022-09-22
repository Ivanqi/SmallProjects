<?php
namespace Core\Http;

class FileBag extends ParameterBag {

    private static $fileKeys = ['error', 'name', 'size', 'tmp_name', 'type'];

    public function __construct(array $parameters = [])
    {
        $this->replace($parameters);
    }

    public function replace(array $files = []): void
    {
        $this->parameters = [];
        $this->add($files);
    }

    public function set(string $key, $value): void
    {
        // if (!is_array($value) && !$value instanceof UploadedFile) {
        //     throw new \InvalidArgumentException('An uploaded file must be an array or an instance of UploadedFile.');
        // }

        // parent::set($key, $this->convertFileInformation($value));
        parent::set($key, $value);
    }

    public function add(array $files = []): void
    {
        foreach ($files as $key => $file) {
            $this->set($key, $file);
        }
    }
    
}