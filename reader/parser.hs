import Data.Word
import Data.Bits
import Data.List
import Data.List.Split

-- Utilities
u32Size = 4

unpackInt :: [Word8] -> Int
unpackInt [] = 0
unpackInt (x:xs) = let upper = unpackInt xs in
				   (.|.) (upper `shift` 8) $ fromIntegral x


-- Type Declarations
data Msg = WheelMsg { btn :: Int, state :: Int } 
		 | ButtonMsg { btn :: Int, pressure :: Int } deriving (Show)
data Packet = Packet { counter :: Int
					 , msgs :: [Msg]
			  		 } deriving (Show)


-- Message Decoding
decodeWheelMsg :: [Int] -> Msg
decodeWheelMsg fields = WheelMsg { btn=(fields !! 0), state=(fields !! 1) }

decodeButtonMsg :: [Int] -> Msg
decodeButtonMsg fields = ButtonMsg { btn=(fields !! 0), pressure=(fields !! 2) }

decodeMsg :: ([Int] -> Msg) -> Int -> [Int] -> Either String Msg
decodeMsg decodeFn numFields fields
	| length fields /= numFields  = Left $ "Message must be " ++ (show numFields) ++ " bytes long"
	| otherwise = Right $ decodeFn fields

isValidMsg :: Either String (Msg) -> Bool
isValidMsg (Left _) = False
isValidMsg (Right _) = True


-- Body Decoding
decodeBody :: ([Int] -> Msg) -> Int -> [Int] -> Either String Packet
decodeBody msgDecodeFn msgLen fields
	| packLen < minPackLen = Left $ "Packet must be at least " ++ (show minPackLen) ++ " bytes long"
	| packMsgsLen `mod` msgLen /= 0 = Left $ "Invalid packet messages length " ++ (show $ packMsgsLen)
	| otherwise = let 
					numFields = msgLen `div` u32Size
					decFn = decodeMsg (msgDecodeFn) numFields
					filtFn = filter (isValidMsg) . map (decFn)
					msgFields = chunksOf numFields $ drop 3 fields in
				  Right $ Packet { counter=(fields !! 0), msgs=([m | Right m <- filtFn msgFields]) }
	where 
		ctrlLen = 12
		packLen = length fields * u32Size
		packMsgsLen = packLen - ctrlLen
		minPackLen = ctrlLen + msgLen


-- Packet Decoding
decode :: [Word8] -> Either String Packet
decode packet
	| packLen < u32Size = Left $ "Packet must be at least " ++ (show u32Size) ++ " bytes"
	| packLen `mod` u32Size /= 0  = Left $ "Packet length must be a multiple of " ++ (show u32Size)
	| otherwise = let 
					(head, body) = splitAt u32Size packet
					packetType = unpackInt head 
					wheelMsgLen = 8
					buttonMsgLen = 12
					bodyFields = map (unpackInt) $ chunksOf u32Size body in
				case packetType of
					0x3504e00 -> decodeBody (decodeButtonMsg) buttonMsgLen bodyFields
					0x3774e00 -> decodeBody (decodeWheelMsg) wheelMsgLen bodyFields
					0x3734e00 -> decodeBody (decodeWheelMsg) wheelMsgLen bodyFields
					_ -> Left "Unknown packet type"
	where packLen = length packet
