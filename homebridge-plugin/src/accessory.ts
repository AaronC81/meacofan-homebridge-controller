import {
  AccessoryConfig,
  AccessoryPlugin,
  API,
  CharacteristicEventTypes,
  CharacteristicGetCallback,
  CharacteristicSetCallback,
  CharacteristicValue,
  HAP,
  Logging,
  Service
} from "homebridge";

import { exec } from "child_process";

/*
 * IMPORTANT NOTICE
 *
 * One thing you need to take care of is, that you never ever ever import anything directly from the "homebridge" module (or the "hap-nodejs" module).
 * The above import block may seem like, that we do exactly that, but actually those imports are only used for types and interfaces
 * and will disappear once the code is compiled to Javascript.
 * In fact you can check that by running `npm run build` and opening the compiled Javascript file in the `dist` folder.
 * You will notice that the file does not contain a `... = require("homebridge");` statement anywhere in the code.
 *
 * The contents of the above import statement MUST ONLY be used for type annotation or accessing things like CONST ENUMS,
 * which is a special case as they get replaced by the actual value and do not remain as a reference in the compiled code.
 * Meaning normal enums are bad, const enums can be used.
 *
 * You MUST NOT import anything else which remains as a reference in the code, as this will result in
 * a `... = require("homebridge");` to be compiled into the final Javascript code.
 * This typically leads to unexpected behavior at runtime, as in many cases it won't be able to find the module
 * or will import another instance of homebridge causing collisions.
 *
 * To mitigate this the {@link API | Homebridge API} exposes the whole suite of HAP-NodeJS inside the `hap` property
 * of the api object, which can be acquired for example in the initializer function. This reference can be stored
 * like this for example and used to access all exported variables and classes from HAP-NodeJS.
 */
let hap: HAP;

/*
 * Initializer function called when the plugin is loaded.
 */
export = (api: API) => {
  hap = api.hap;
  api.registerAccessory("Meacofan1056Tiny", Meacofan1056Tiny);
};

class Meacofan1056Tiny implements AccessoryPlugin {
  private readonly log: Logging;
  private readonly name: string;

  private readonly fanService: Service;
  private readonly informationService: Service;

  private fanOn: boolean = false;
  
  private fanSpeed: number = 0; // 0-100

  constructor(log: Logging, config: AccessoryConfig, api: API) {
    this.log = log;
    this.name = config.name;

    this.fanService = new hap.Service.Fan(this.name);
    this.fanService.getCharacteristic(hap.Characteristic.On)
      .on(CharacteristicEventTypes.GET, (callback: CharacteristicGetCallback) => {
        callback(undefined, this.fanOn);
      })
      .on(CharacteristicEventTypes.SET, (value: CharacteristicValue, callback: CharacteristicSetCallback) => {
        log.info("On SET");
        this.fanOn = value as boolean;
        this.sendState();
        callback();
      });
    this.fanService.getCharacteristic(hap.Characteristic.RotationSpeed)
      .on(CharacteristicEventTypes.GET, (callback: CharacteristicGetCallback) => {
        callback(undefined, this.fanSpeed);
      })
      .on(CharacteristicEventTypes.SET, (value: CharacteristicValue, callback: CharacteristicSetCallback) => {
        log.info("Speed SET");
        this.fanSpeed = value as number;
        this.sendState();
        callback();
      });

    this.informationService = new hap.Service.AccessoryInformation()
      .setCharacteristic(hap.Characteristic.Manufacturer, "Meaco")
      .setCharacteristic(hap.Characteristic.Model, "MeacoFan 1056 with ATtiny85-based I2C adapter");

    log.info("Fan initialised");
  }

  mappedSpeed(): number {
    // Remaps 0-100 into 1-12
    return Math.floor((this.fanSpeed / 100) * 11) + 1;
  }

  sendState() {
    // Compose packet
    let data = 0b00010000;
    if (this.fanOn) {
      data |= this.mappedSpeed();
    }

    // Add checksum
    let calculated_checksum = 0;
    calculated_checksum += (data & 0b00010000 ? 1 : 0);
    calculated_checksum += (data & 0b00001000 ? 1 : 0);
    calculated_checksum += (data & 0b00000100 ? 1 : 0);
    calculated_checksum += (data & 0b00000010 ? 1 : 0);
    calculated_checksum += (data & 0b00000001 ? 1 : 0);
    data |= calculated_checksum << 5;

    // Send a few times to make up for our crap bitbanged I2C
    let command = "i2ctransfer -y 1 w1@0x40 0x" + data.toString(16);
    this.log.info("Executing: " + command);
    for (var i = 0; i < 5; i++) {
      setTimeout(() => {
        exec(command);
      }, i * 100);
    }
  }

  /*
   * This method is called directly after creation of this instance.
   * It should return all services which should be added to the accessory.
   */
  getServices(): Service[] {
    return [
      this.informationService,
      this.fanService,
    ];
  }
}
