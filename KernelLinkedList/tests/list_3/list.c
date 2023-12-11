/*
 * @Author: ivanqi
 * @Date: 2023-12-11 22:24:02
 * @LastEditors: ivanqi
 * @LastEditTime: 2023-12-11 22:31:05
 * @Description: 
 */
#include "list.h"
#include <stdio.h>
#include <string.h>

#define MAX_NAME_LEN 32
#define MAX_ID_LEN 10

struct list_head device_list;

#define I2C_TYPE 1
#define SPI_TYPE 2

char *dev_name[] = {
  "none",
  "I2C",
  "SPI"
};

struct device {
  int type;
  char name[MAX_NAME_LEN];
  struct list_head list;
};

struct i2c_node {
  int data;
  unsigned int reg;
  struct device dev;
};

struct spi_node {  
  unsigned int reg;
  struct device dev;
};

void display_i2c_device(struct device *device) {
  struct i2c_node *i2c_device = container_of(device, struct i2c_node, dev);

  printf("\t  i2c_device->data: %d\n", i2c_device->data);
  printf("\t  i2c_device->reg: %#x\n", i2c_device->reg);
}

void display_spi_device(struct device *device) {
  struct spi_node *spi_device = container_of(device, struct spi_node, dev);

  printf("\t  spi_device->reg: %#x\n",spi_device->reg);
}

void display_device(struct device *device) {
    printf("\t  dev.type: %d\n", device->type);
    printf("\t  dev.type: %s\n", dev_name[device->type]);
    printf("\t  dev.name: %s\n", device->name);
}

void display_list(struct list_head *list_head) {
  int i = 0;
  struct list_head *p;
  struct device *entry;

  printf("-------list---------\n");
  list_for_each(p, list_head) {
    printf("node[%d]\n",i++);
    entry = list_entry(p, struct device,list);

    switch(entry->type) {
      case I2C_TYPE:
        display_i2c_device(entry);
        break;
      case SPI_TYPE:
        display_spi_device(entry);
        break;
      default:
        printf("unknown device type!\n");
        break;
    }

    display_device(entry);
  }

  printf("-------end----------\n");
}

void i2c_register_device(struct device *dev) {
  struct i2c_node *i2c_device = container_of(dev, struct i2c_node, dev);

  i2c_device->dev.type = I2C_TYPE;
  strcpy(i2c_device->dev.name, "ivan1");

  list_add(&dev->list, &device_list);
}

void spi_register_device(struct device *dev) {
  struct spi_node *spi_device=container_of(dev, struct spi_node, dev);

  spi_device->dev.type = SPI_TYPE;
  strcpy(spi_device->dev.name, "ivan2");

  list_add(&dev->list, &device_list);  
}

void i2c_unregister_device(struct device *dev) {
  struct i2c_node *i2c_device = container_of(dev, struct i2c_node, dev);

  list_del(&dev->list);
}

void spi_unregister_device(struct device *dev) {
  struct spi_node *spi_device = container_of(dev, struct spi_node, dev);

  list_del(&dev->list);
}

int main(void) {

  struct i2c_node dev1;
  struct spi_node dev2;

  INIT_LIST_HEAD(&device_list);
  dev1.data = 1;
  dev1.reg = 0x40009000;
  i2c_register_device(&dev1.dev);

  dev2.reg  = 0x40008000;
  spi_register_device(&dev2.dev);  

  display_list(&device_list);
  spi_unregister_device(&dev1.dev);
  display_list(&device_list);  

  return 0;
}