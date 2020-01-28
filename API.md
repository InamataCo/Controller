# MQTT API Documentation

## Objects Interface

### Add: `objects/<uuid>/add`

The add command creates a new periphery. It has to be called before the execute command can be sent to a created periphery. Each add command has to have the following JSON parameters in addition to peripheral specific parameters.

| parameter | content                       |
| --------- | ----------------------------- |
| name      | unique name of the peripheral |
| type      | type of the peripheral        |

#### Dummy Periphery

No extra parameters required

#### I2C Adapter Periphery

| parameter | content          |
| --------- | ---------------- |
| scl       | clock signal pin |
| sda       | data signal pin  |

#### BH1750 Periphery

| parameter   | content                           |
| ----------- | --------------------------------- |
| i2c_adapter | name of the I2C adapter periphery |
| address     | address of the BH1750 sensor      |

Note: The I2C adapter has to have been added

### Delete: `objects/<uuid>/delete`

### Execute: `objects/<uuid>/execute`

## Tasks

### L293D Motors

The L293D motor drivers are used to power motors with up to 21 W (36V / 600mA). They require two pins to define the turning direction and one to enable and set the speed.

| action  | topic                        | parameters                                             | Note                                                                                                  |
| ------- | ---------------------------- | ------------------------------------------------------ | ----------------------------------------------------------------------------------------------------- |
| add     | `action/<uuid>/l293d_motors` | name <br> pin_forward <br> pin_reverse <br> pin_enable | The name has to be unique                                                                             |
| remove  | `action/<uuid>/l293d_motors` | name                                                   |                                                                                                       |
| enable  | `action/<uuid>/l293d_motors` | name <br> power_percent <br> forward <br> duration_s   | percent as integer (0-100) <br> forward as bool, false is reverse <br> duration is time to be enabled |
| disable | `action/<uuid>/l293d_motors` | name                                                   |                                                                                                       |
