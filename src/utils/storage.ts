// Storage utilities with error handling

export const saveToLocalStorage = <T>(key: string, data: T): boolean => {
    try {
        localStorage.setItem(key, JSON.stringify(data))
        return true
    } catch (error) {
        console.error('Error saving to localStorage:', error)
        return false
    }
}

export const loadFromLocalStorage = <T>(key: string): T | null => {
    try {
        const saved = localStorage.getItem(key)
        if (!saved) return null
        return JSON.parse(saved) as T
    } catch (error) {
        console.error('Error loading from localStorage:', error)
        return null
    }
}

export const removeFromLocalStorage = (key: string): boolean => {
    try {
        localStorage.removeItem(key)
        return true
    } catch (error) {
        console.error('Error removing from localStorage:', error)
        return false
    }
}

export const clearLocalStorage = (): boolean => {
    try {
        localStorage.clear()
        return true
    } catch (error) {
        console.error('Error clearing localStorage:', error)
        return false
    }
}

export const getAllKeys = (prefix?: string): string[] => {
    try {
        const keys = Object.keys(localStorage)
        return prefix ? keys.filter(key => key.startsWith(prefix)) : keys
    } catch (error) {
        console.error('Error getting localStorage keys:', error)
        return []
    }
}
