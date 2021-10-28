package field

// 拥有bool类型的值的日志字段
type boolField struct {
	name string
	fieldType FieldType
	value bool
}

func (field *boolField) Name() string {
	return field.name
}

func (field *boolField) Type() FieldType {
	return field.fieldType
}

func (field *boolField) Value() interface{} {
	return field.value
}

func Bool (name string, value bool) Field {
	return &boolField{name: name, fieldType: BoolType, value: value}
}

// 拥有int64类型的值的日志字段
type int64Field struct {
	name string
	fieldType FieldType
	value int64
}

func (field *int64Field) Name() string {
	return field.name
}

func (field *int64Field) Type() FieldType {
	return field.fieldType
}

func Int64(name string, value int64) Field {
	return &int64Field{name: name, fieldType: int64Field, value: value}
}

// 拥有float64 类型的值的日志字段
type float64Field struct {
	name string
	fieldType FieldType
	value float64
}

func (field *float64Field) Name() string {
	return field.name
}

func (field *float64Field) Type() FieldType {
	return field.fieldType
}

func (field *float64Field) Value() interface{} {
	return field.value
}

func Float64(name string, value float64) Field {
	return &float64Field{name: name, fieldType: Float64Type, value: value}
}

// 拥有string类型的值的日志字段。
type stringField struct {
	name      string
	fieldType FieldType
	value     string
}

func (field *stringField) Name() string {
	return field.name
}

func (field *stringField) Type() FieldType {
	return field.fieldType
}

func (field *stringField) Value() interface{} {
	return field.value
}

func String(name string, value string) Field {
	return &stringField{name: name, fieldType: StringType, value: value}
}

// 拥有interface{}类型的值的日志字段。
type objectField struct {
	name      string
	fieldType FieldType
	value     interface{}
}

func (field *objectField) Name() string {
	return field.name
}

func (field *objectField) Type() FieldType {
	return field.fieldType
}

func (field *objectField) Value() interface{} {
	return field.value
}

func Object(name string, value interface{}) Field {
	return &objectField{name: name, fieldType: ObjectType, value: value}
}