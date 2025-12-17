import { useState, useEffect } from 'react'
import type { Reference } from '../types'

// Custom hook for managing references data

export function useReferences() {
    const [references, setReferences] = useState<Reference[]>([])
    const [loading, setLoading] = useState(true)
    const [error, setError] = useState<string | null>(null)

    useEffect(() => {
        const fetchReferences = async () => {
            try {
                setLoading(true)
                setError(null)
                const response = await fetch('/references.json')

                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`)
                }

                const data = await response.json()
                setReferences(data)
            } catch (err) {
                const errorMessage = err instanceof Error ? err.message : 'Error desconocido'
                setError(`Error cargando referencias: ${errorMessage}`)
                console.error('Error loading references:', err)
            } finally {
                setLoading(false)
            }
        }

        fetchReferences()
    }, [])

    return { references, loading, error, refetch: () => setReferences([]) }
}
