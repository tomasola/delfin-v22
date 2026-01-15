import type { Reference } from '../types'

// Search utilities

// Normalize code: remove dots and special characters, keep only alphanumeric and hyphens
const normalizeCode = (code: string): string => {
    return code.replace(/[.\s]/g, '').toLowerCase().trim()
}

export const filterReferences = (
    references: Reference[],
    searchTerm: string,
    searchInCategory: boolean = true
): Reference[] => {
    if (!searchTerm.trim()) return references

    const searchLower = searchTerm.toLowerCase().trim()
    const normalizedSearch = normalizeCode(searchTerm)

    return references.filter(ref => {
        // Original exact match (for backward compatibility)
        const codeMatch = ref.code.toLowerCase().includes(searchLower)

        // Normalized match (e.g., "10008" matches "10.008")
        const normalizedCodeMatch = normalizeCode(ref.code).includes(normalizedSearch)

        const categoryMatch = searchInCategory && ref.category.toLowerCase().includes(searchLower)

        return codeMatch || normalizedCodeMatch || categoryMatch
    })
}

export const getUniqueCategories = (references: Reference[]): string[] => {
    const categories = new Set(references.map(ref => ref.category))
    return Array.from(categories).sort()
}

export const filterByCategory = (
    references: Reference[],
    category: string
): Reference[] => {
    if (!category) return references
    return references.filter(ref => ref.category.toLowerCase() === category.toLowerCase())
}

export const highlightSearchTerm = (text: string, searchTerm: string): string => {
    if (!searchTerm.trim()) return text

    const regex = new RegExp(`(${searchTerm})`, 'gi')
    return text.replace(regex, '<mark>$1</mark>')
}
