// Web Bluetooth API types
interface BluetoothDevice extends EventTarget {
    id: string;
    name?: string;
    gatt?: BluetoothRemoteGATTServer;
}

interface BluetoothRemoteGATTServer {
    device: BluetoothDevice;
    connected: boolean;
    connect(): Promise<BluetoothRemoteGATTServer>;
    disconnect(): void;
    getPrimaryService(service: string | number): Promise<BluetoothRemoteGATTService>;
}

interface BluetoothRemoteGATTService extends EventTarget {
    device: BluetoothDevice;
    uuid: string;
    isPrimary: boolean;
    getCharacteristic(characteristic: string | number): Promise<BluetoothRemoteGATTCharacteristic>;
}

interface BluetoothRemoteGATTCharacteristic extends EventTarget {
    service: BluetoothRemoteGATTService;
    uuid: string;
    properties: BluetoothCharacteristicProperties;
    value?: DataView;
    readValue(): Promise<DataView>;
    writeValue(value: BufferSource): Promise<void>;
    writeValueWithResponse(value: BufferSource): Promise<void>;
    startNotifications(): Promise<BluetoothRemoteGATTCharacteristic>;
    stopNotifications(): Promise<BluetoothRemoteGATTCharacteristic>;
}

interface BluetoothCharacteristicProperties {
    authenticatedSignedWrites: boolean;
    broadcast: boolean;
    extendedProperties: boolean;
    indicate: boolean;
    notify: boolean;
    read: boolean;
    reliableWrite: boolean;
    writableAuxiliaries: boolean;
    write: boolean;
    writeWithoutResponse: boolean;
}

interface Bluetooth extends EventTarget {
    requestDevice(options?: RequestDeviceOptions): Promise<BluetoothDevice>;
    getAvailability(): Promise<boolean>;
}

interface RequestDeviceOptions {
    filters?: Array<BluetoothRequestDeviceFilter>;
    optionalServices?: Array<string | number>;
    acceptAllDevices?: boolean;
}

interface BluetoothRequestDeviceFilter {
    name?: string;
    namePrefix?: string;
    services?: Array<string | number>;
}

interface Navigator {
    bluetooth: Bluetooth;
}
