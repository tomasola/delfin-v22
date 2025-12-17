
import { useState, useMemo } from 'react'

interface CategorySelectorModalProps {
    categories: string[]
    onSelect: (category: string) => void
    onClose: () => void
    isOpen: boolean
}

export function CategorySelectorModal({ categories, onSelect, onClose, isOpen }: CategorySelectorModalProps) {
    const [searchTerm, setSearchTerm] = useState('')

    const filteredCategories = useMemo(() => {
        return categories.filter(c =>
            c.toLowerCase().includes(searchTerm.toLowerCase())
        )
    }, [categories, searchTerm])

    if (!isOpen) return null

    return (
        <div className="fixed inset-0 z-50 flex items-end sm:items-center justify-center bg-black/60 backdrop-blur-sm p-4 animate-in fade-in duration-200">
            <div className="bg-white dark:bg-gray-900 w-full max-w-lg rounded-t-2xl sm:rounded-2xl shadow-2xl h-[85vh] sm:h-[70vh] flex flex-col overflow-hidden animate-in slide-in-from-bottom duration-300">

                {/* Header */}
                <div className="p-4 border-b border-gray-200 dark:border-gray-800 flex justify-between items-center bg-gray-50 dark:bg-gray-900/50">
                    <h2 className="text-lg font-bold text-gray-800 dark:text-white flex items-center gap-2">
                        📂 Seleccionar Carpeta
                    </h2>
                    <button
                        onClick={onClose}
                        className="p-2 rounded-full hover:bg-gray-200 dark:hover:bg-gray-800 text-gray-500 transition-colors"
                    >
                        ✕
                    </button>
                </div>

                {/* Search Input */}
                <div className="p-4 border-b border-gray-100 dark:border-gray-800 bg-white dark:bg-gray-900 sticky top-0 z-10">
                    <input
                        type="text"
                        placeholder="🔎 Buscar carpeta..."
                        value={searchTerm}
                        onChange={(e) => setSearchTerm(e.target.value)}
                        className="w-full px-4 py-3 rounded-xl bg-gray-100 dark:bg-gray-800 border-none focus:ring-2 focus:ring-blue-500 text-gray-800 dark:text-white placeholder-gray-500 outline-none text-base"
                        autoFocus
                    />
                </div>

                {/* Categories List */}
                <div className="flex-1 overflow-y-auto p-2 scrollbar-thin scrollbar-thumb-gray-300 dark:scrollbar-thumb-gray-700">
                    {filteredCategories.length > 0 ? (
                        <div className="grid grid-cols-1 gap-1">
                            {filteredCategories.map(category => (
                                <button
                                    key={category}
                                    onClick={() => {
                                        onSelect(category)
                                        onClose()
                                    }}
                                    className="w-full text-left px-4 py-3 rounded-lg hover:bg-blue-50 dark:hover:bg-blue-900/30 text-gray-700 dark:text-gray-300 font-medium transition-colors border-b border-gray-50 dark:border-gray-800 last:border-0 flex items-center gap-3"
                                >
                                    <span className="text-xl">📁</span>
                                    {category}
                                </button>
                            ))}
                        </div>
                    ) : (
                        <div className="text-center py-10 text-gray-500 dark:text-gray-400">
                            <p className="text-4xl mb-2">😕</p>
                            <p>No se encontraron carpetas</p>
                        </div>
                    )}
                </div>

                {/* Footer */}
                <div className="p-4 border-t border-gray-200 dark:border-gray-800 bg-gray-50 dark:bg-gray-900/50 text-center">
                    <span className="text-xs text-gray-500">
                        {filteredCategories.length} carpetas encontradas
                    </span>
                </div>
            </div>
        </div>
    )
}
