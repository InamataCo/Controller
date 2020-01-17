# MQTT API Documentation

## Tasks

### L293D Motors

The L293D motor drivers are used to power motors with up to 21 W (36V / 600mA). They require two pins to define the turning direction and one to enable and set the speed.

| action | topic | parameters | Note |
| ------ | ----- | ---------- | ---- |
| add | `action/<uuid>/l293d_motors` | name <br> pin_forward <br> pin_reverse <br> pin_enable | The name has to be unique |
| remove | `action/<uuid>/l293d_motors` | name | |
| enable | `action/<uuid>/l293d_motors` | name <br> power_percent <br> forward <br> duration_s | percent as integer (0-100) <br> forward as bool, false is reverse <br> duration is time to be enabled |
| disable | `action/<uuid>/l293d_motors` | name | |

