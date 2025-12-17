import type { Reference } from '../types'

// Search utilities

export const filterReferences = (
    references: Reference[],
    searchTerm: string,
    searchInCategory: boolean = true
): Reference[] => {
    if (!searchTerm.trim()) return references

    const searchLower = searchTerm.toLowerCase().trim()

    return references.filter(ref => {
        const codeMatch = ref.code.toLowerCase().includes(searchLower)
        const categoryMatch = searchInCategory && ref.category.toLowerCase().includes(searchLower)
        return codeMatch || categoryMatch
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
