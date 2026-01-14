// Type definitions for Delfín Etiquetas

export interface Reference {
    code: string
    image: string
    category: string
    embedding?: number[]
}

export interface ReferenceData {
    length: string
    quantity: string
    boxSize: string
    notes: string
}

export interface ToastMessage {
    id: string
    message: string
    type: 'success' | 'error' | 'info' | 'warning'
    duration?: number
}

export interface SearchHistory {
    term: string
    timestamp: number
}

export interface AppTheme {
    mode: 'light' | 'dark'
}
