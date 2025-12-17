export function LoadingSpinner() {
    return (
        <div className="flex flex-col items-center justify-center min-h-[400px]">
            <div className="relative w-16 h-16">
                <div className="absolute top-0 left-0 w-full h-full border-4 border-blue-200 rounded-full"></div>
                <div className="absolute top-0 left-0 w-full h-full border-4 border-blue-500 rounded-full border-t-transparent animate-spin"></div>
            </div>
            <p className="mt-4 text-gray-600 dark:text-gray-400">Cargando referencias...</p>
        </div>
    )
}

export function LoadingSpinnerSmall() {
    return (
        <div className="inline-block w-5 h-5 border-2 border-blue-500 border-t-transparent rounded-full animate-spin"></div>
    )
}
