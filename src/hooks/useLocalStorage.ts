import { useState } from 'react'
import { saveToLocalStorage, loadFromLocalStorage } from '../utils/storage'

// Custom hook for localStorage with type safety and error handling

export function useLocalStorage<T>(key: string, initialValue: T): [T, (value: T) => void, () => void] {
    // State to store our value
    const [storedValue, setStoredValue] = useState<T>(() => {
        const item = loadFromLocalStorage<T>(key)
        return item !== null ? item : initialValue
    })

    // Return a wrapped version of useState's setter function that persists to localStorage
    const setValue = (value: T) => {
        setStoredValue(value)
        saveToLocalStorage(key, value)
    }

    // Function to remove the value from localStorage
    const removeValue = () => {
        setStoredValue(initialValue)
        try {
            localStorage.removeItem(key)
        } catch (error) {
            console.error('Error removing from localStorage:', error)
        }
    }

    return [storedValue, setValue, removeValue]
}
