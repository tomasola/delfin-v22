import type { ReferenceData } from '../types'
import { getAllKeys, loadFromLocalStorage } from './storage'

// Export utilities

interface ExportData {
    code: string
    length: string
    quantity: string
    boxSize: string
    notes: string
}

export const exportToCSV = (): string => {
    const keys = getAllKeys('ref_')
    const data: ExportData[] = []

    keys.forEach(key => {
        const refData = loadFromLocalStorage<ReferenceData>(key)
        if (refData) {
            data.push({
                code: key.replace('ref_', ''),
                ...refData
            })
        }
    })

    if (data.length === 0) return ''

    // CSV header
    const headers = ['Código', 'Longitud (m)', 'Cantidad', 'Tamaño de Caja', 'Notas']
    const csvRows = [headers.join(',')]

    // CSV data
    data.forEach(item => {
        const row = [
            item.code,
            item.length,
            item.quantity,
            item.boxSize,
            `"${item.notes.replace(/"/g, '""')}"` // Escape quotes
        ]
        csvRows.push(row.join(','))
    })

    return csvRows.join('\n')
}

export const exportToJSON = (): string => {
    const keys = getAllKeys('ref_')
    const data: Record<string, ReferenceData> = {}

    keys.forEach(key => {
        const refData = loadFromLocalStorage<ReferenceData>(key)
        if (refData) {
            data[key.replace('ref_', '')] = refData
        }
    })

    return JSON.stringify(data, null, 2)
}

export const downloadFile = (content: string, filename: string, mimeType: string = 'text/plain') => {
    const blob = new Blob([content], { type: mimeType })
    const url = URL.createObjectURL(blob)
    const link = document.createElement('a')
    link.href = url
    link.download = filename
    document.body.appendChild(link)
    link.click()
    document.body.removeChild(link)
    URL.revokeObjectURL(url)
}

export const exportDataAsCSV = () => {
    const csv = exportToCSV()
    if (csv) {
        downloadFile(csv, `delfin-export-${new Date().toISOString().split('T')[0]}.csv`, 'text/csv')
        return true
    }
    return false
}

export const exportDataAsJSON = () => {
    const json = exportToJSON()
    if (json !== '{}') {
        downloadFile(json, `delfin-export-${new Date().toISOString().split('T')[0]}.json`, 'application/json')
        return true
    }
    return false
}
