import { useState } from 'react';

interface RobustImageProps {
    code: string;
    className?: string;
    style?: React.CSSProperties;
    priority?: 'jpg' | 'bmp';
}

export const RobustImage = ({ code, className, style, priority = 'jpg' }: RobustImageProps) => {
    const getSafeImageSrc = (code: string, p: 'jpg' | 'bmp') => {
        return p === 'jpg'
            ? [`/images/perfiles/${code}.jpg`, `/images/perfiles/${code}.bmp`, `/images/${code}.jpg`, `/images/${code}.bmp`]
            : [`/images/perfiles/${code}.bmp`, `/images/perfiles/${code}.jpg`, `/images/${code}.bmp`, `/images/${code}.jpg`];
    };

    const [srcIdx, setSrcIdx] = useState(0);
    const sources = getSafeImageSrc(code, priority);

    return (
        <img
            src={sources[srcIdx]}
            onError={() => {
                if (srcIdx < sources.length - 1) {
                    setSrcIdx(srcIdx + 1);
                }
            }}
            alt={code}
            className={className}
            style={style}
            loading="lazy"
        />
    );
};
