
import { useState } from 'react'
import { CategorySelectorModal } from './CategorySelectorModal'

interface CategoryFilterProps {
    categories: string[]
    selectedCategory: string
    onSelectCategory: (category: string) => void
}

export function CategoryFilter({ categories, selectedCategory, onSelectCategory }: CategoryFilterProps) {
    const [isModalOpen, setIsModalOpen] = useState(false)

    // Filter out Perfiles from the generic list since it has its own button
    const otherCategories = categories.filter(c => c.toUpperCase() !== 'PERFILES')

    const currentLabel = selectedCategory && selectedCategory !== 'PERFILES'
        ? selectedCategory
        : '📂 Ver Carpetas...'

    return (
        <div className="mb-6">
            <h3 className="text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">Filtrar:</h3>
            <div className="flex flex-wrap gap-4 items-center">
                {/* Perfiles Button (Primary - Always Visible) */}
                <button
                    onClick={() => onSelectCategory('PERFILES')}
                    className={`flex items-center gap-2 px-6 py-3 rounded-xl text-sm font-bold transition-all transform active:scale-95 ${selectedCategory === 'PERFILES'
                        ? 'bg-gradient-to-r from-blue-600 to-blue-500 text-white shadow-lg shadow-blue-500/30 ring-2 ring-blue-300'
                        : 'bg-white dark:bg-gray-800 text-gray-700 dark:text-gray-200 border border-gray-200 dark:border-gray-700 hover:border-blue-500 shadow-sm'
                        }`}
                >
                    📁 PERFILES
                </button>

                {/* Open Modal Button */}
                <button
                    onClick={() => setIsModalOpen(true)}
                    className={`flex items-center gap-2 px-6 py-3 rounded-xl text-sm font-medium transition-all transform active:scale-95 flex-1 md:flex-none justify-between md:justify-start ${(selectedCategory && selectedCategory !== 'PERFILES')
                        ? 'bg-blue-50 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300 border border-blue-200 dark:border-blue-700'
                        : 'bg-white dark:bg-gray-800 text-gray-700 dark:text-gray-200 border border-gray-200 dark:border-gray-700 hover:border-gray-400'
                        }`}
                >
                    <span className="truncate max-w-[150px]">{currentLabel}</span>
                    <span className="text-gray-400">▼</span>
                </button>

                {/* Modal */}
                <CategorySelectorModal
                    categories={otherCategories}
                    isOpen={isModalOpen}
                    onClose={() => setIsModalOpen(false)}
                    onSelect={onSelectCategory}
                />

                {/* Clear Filter Button */}
                {selectedCategory && (
                    <button
                        onClick={() => onSelectCategory('')}
                        className="px-4 py-2 bg-red-50 dark:bg-red-900/20 text-red-600 dark:text-red-400 rounded-lg hover:bg-red-100 dark:hover:bg-red-900/40 transition-colors font-bold text-sm shadow-sm border border-red-200 dark:border-red-800"
                    >
                        ✕ Limpiar
                    </button>
                )}
            </div>
        </div>
    )
}
