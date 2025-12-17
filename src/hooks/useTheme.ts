import { useEffect } from 'react'
import { useLocalStorage } from './useLocalStorage'

// Custom hook for theme management (light/dark mode)

export type Theme = 'light' | 'dark'

export function useTheme() {
    const [theme, setTheme] = useLocalStorage<Theme>('theme', 'light')

    useEffect(() => {
        const root = document.documentElement

        if (theme === 'dark') {
            root.classList.add('dark')
        } else {
            root.classList.remove('dark')
        }
    }, [theme])

    const toggleTheme = () => {
        setTheme(theme === 'light' ? 'dark' : 'light')
    }

    return { theme, setTheme, toggleTheme }
}
